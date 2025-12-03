import {
  modalBackdropTemplate,
  modalTemplate,
  pillTemplate,
  commentCardTemplate
} from "./templates.js";
import { patchIssueFields, fetchUsers, createIssue, mapIssue, apiClient } from "./api.js";

const pillConfig = [
  { label: "Assignees", className: "assignees", value: (issue) => issue.assignedTo || "" },
  { label: "Tags", className: "tags", value: (issue) => (issue.tags || []).map((t) => t.label).join(", ") },
  { label: "Milestone", className: "milestone", value: (issue) => issue.milestone || "" },
  { label: "Status", className: "status", value: (issue) => issue.status || issue.milestone || "" }
];

const setText = (el, value, fallback = "") => {
  if (!el) return;
  el.textContent = value || fallback;
};

const emptyIssue = {
  rawId: null,
  id: "",
  title: "",
  description: "",
  status: "To Be Done",
  comments: [],
  tags: []
};

export const createModal = ({ onIssueUpdated, getActiveDatabase } = {}) => {
  const backdrop = modalBackdropTemplate();
  document.body.appendChild(backdrop);
  let currentIssue = null;
  let workingIssue = null;
  let usersCache = null;

  const setStatus = (modal, message, isError = false) => {
    const box = modal.querySelector('[data-role="form-status"]');
    if (!box) return;
    box.textContent = message || "";
    box.classList.toggle("error", isError);
  };

  const fillView = (modal, issue) => {
    const heading = issue.id ? `${issue.title} (${issue.id})` : issue.title || "Issue";
    setText(modal.querySelector('[data-field="heading"]'), heading, "Issue");
    setText(modal.querySelector('[data-field="titleBox"]'), heading, "Issue");
    setText(
      modal.querySelector('[data-field="description"]'),
      issue.description,
      "No description provided."
    );

    const sidebar = modal.querySelector('[data-role="sidebar"]');
    sidebar.replaceChildren();
    pillConfig.forEach((pill) => {
      const el = pillTemplate(pill.label, pill.className);
      const value = pill.value ? pill.value(issue) : "";
      el.textContent = value ? `${pill.label}: ${value}` : pill.label;
      sidebar.appendChild(el);
    });

    const commentsWrap = modal.querySelector('[data-role="comments"]');
    commentsWrap.replaceChildren();
    (issue.comments || []).forEach((comment) => {
      commentsWrap.appendChild(commentCardTemplate(comment));
    });
  };

  const bindEditHandlers = (modal) => {
    const titleBox = modal.querySelector('[data-field="titleBox"]');
    const heading = modal.querySelector('[data-field="heading"]');
    const descriptionBox = modal.querySelector('[data-field="description"]');
    const authorWrap = modal.querySelector('[data-role="author-select"]');

    const populateAuthor = (users) => {
      if (!authorWrap) return;
      authorWrap.innerHTML = "";

      if (!Array.isArray(users) || users.length === 0) {
        authorWrap.textContent = "No users available. Please add a user.";
        authorWrap.classList.add("error");
        return;
      }

      const label = document.createElement("span");
      label.textContent = "Author";
      const select = document.createElement("select");
      users.forEach((u) => {
        const name = u.name || u.id || u;
        const opt = document.createElement("option");
        opt.value = name;
        opt.textContent = name;
        select.appendChild(opt);
      });

      const currentAuthor = workingIssue.author || workingIssue.authorId;
      const defaultAuthor = users[0].name || users[0].id || users[0];
      const resolvedAuthor = currentAuthor || defaultAuthor;
      workingIssue.author = resolvedAuthor;
      select.value = resolvedAuthor;

      select.addEventListener("change", (evt) => {
        workingIssue.author = evt.target.value;
        setStatus(modal, "Pending save (applies when closing).");
      });

      authorWrap.appendChild(label);
      authorWrap.appendChild(select);
    };

    const startInlineEdit = (field, el) => {
      if (!el) return;

      const original =
        field === "title"
          ? (workingIssue.title || "")
          : (workingIssue.description || "");

      // Temporarily show raw value (without the id suffix) for title edits.
      if (field === "title") {
        el.textContent = original;
        setText(heading, original || "Issue");
      } else {
        el.textContent = original;
      }

      el.contentEditable = "true";
      el.classList.add("editing-inline");
      setStatus(modal, `Editing ${field}. Changes save when you close.`);
      const selectAll = () => {
        const range = document.createRange();
        range.selectNodeContents(el);
        const sel = window.getSelection();
        sel.removeAllRanges();
        sel.addRange(range);
      };
      selectAll();

      const cleanup = () => {
        el.contentEditable = "false";
        el.classList.remove("editing-inline");
        el.removeEventListener("keydown", onKeyDown);
        el.removeEventListener("blur", onBlur);
      };

      const cancel = () => {
        cleanup();
        fillView(modal, workingIssue);
        setStatus(modal, "");
      };

      const saveDraft = () => {
        const next = el.textContent.trim();
        if (field === "title" && next.length === 0) {
          setStatus(modal, "Title cannot be empty.", true);
          el.focus();
          return;
        }
        if (next === original) {
          cancel();
          return;
        }

        if (field === "title") {
          workingIssue.title = next;
        } else {
          workingIssue.description = next;
        }
        fillView(modal, workingIssue);
        setStatus(modal, "Pending save (applies when closing).");
        cleanup();
      };

      const onKeyDown = (evt) => {
        if (evt.key === "Escape") {
          evt.preventDefault();
          cancel();
        } else if (evt.key === "Enter" && (field === "title" || evt.ctrlKey)) {
          evt.preventDefault();
          el.blur();
        }
      };

      const onBlur = () => {
        saveDraft();
      };

      el.addEventListener("keydown", onKeyDown);
      el.addEventListener("blur", onBlur, { once: true });
    };

    const startTitleEdit = () => startInlineEdit("title", titleBox);
    const startDescEdit = () => startInlineEdit("description", descriptionBox);
    heading?.addEventListener("dblclick", startTitleEdit);
    titleBox?.addEventListener("dblclick", startTitleEdit);
    descriptionBox?.addEventListener("dblclick", startDescEdit);

    // hydrate author selector once users are loaded
    if (usersCache) {
      populateAuthor(usersCache);
    } else {
      fetchUsers()
        .then((users) => {
          usersCache = users;
          populateAuthor(users);
        })
        .catch((err) => {
          authorWrap.textContent = err.message || "Failed to load users.";
          authorWrap.classList.add("error");
        });
    }
  };

  const buildDetail = (issue) => {
    const modal = modalTemplate();
    modal.dataset.issueId = issue.rawId ?? "";
    currentIssue = { ...apiClient.mapIssue(issue) };
    workingIssue = { ...currentIssue };
    if (!workingIssue.database) {
      workingIssue.database =
        (getActiveDatabase && getActiveDatabase()) || apiClient.getActiveDatabaseName();
    }
    fillView(modal, workingIssue);
    bindEditHandlers(modal);
    const cancelBtn = modal.querySelector('[data-role="cancel-modal"]');
    cancelBtn?.addEventListener("click", () => {
      void close(true);  // discard any changes
    });
    modal.querySelector(".modal-close")?.addEventListener("click", () => {
      void close(true);
    });
    setStatus(modal, "");
    return modal;
  };

  const persistChanges = async (modal) => {
    if (!workingIssue) return true;

    const needsCreate = workingIssue.rawId === undefined || workingIssue.rawId === null;
    if (needsCreate) {
      const title = (workingIssue.title || "").trim();
      if (title.length === 0) {
        setStatus(modal, "Title is required to create an issue.", true);
        return false;
      }
      const authorId =
        (workingIssue.author || workingIssue.authorId || (usersCache && usersCache[0] && (usersCache[0].name || usersCache[0].id || usersCache[0])) || "").trim();
      if (!authorId) {
        setStatus(modal, "Author is required to create an issue.", true);
        return false;
      }
      try {
        setStatus(modal, "Creating issue...");
        const created = await createIssue({
          title,
          description: workingIssue.description || "",
          authorId
        });
        const dbName =
          (getActiveDatabase && getActiveDatabase()) || apiClient.getActiveDatabaseName();
        const mapped = mapIssue(created, dbName);
        workingIssue = {
          ...workingIssue,
          ...mapped,
          rawId: Number(mapped.rawId ?? mapped.id ?? workingIssue.rawId),
          database: mapped.database || dbName,
          author: mapped.author || workingIssue.author
        };
        currentIssue = { ...workingIssue };
        if (onIssueUpdated) {
          onIssueUpdated(workingIssue);
        }
        return true;
      } catch (err) {
        setStatus(modal, err.message || "Failed to create issue.", true);
        return false;
      }
    }

    const payload = {};
    if (workingIssue.title !== currentIssue.title) {
      payload.title = workingIssue.title;
    }
    if (workingIssue.description !== currentIssue.description) {
      payload.description = workingIssue.description;
    }

    if (Object.keys(payload).length === 0) {
      return true;
    }

    try {
      setStatus(modal, "Saving changes...");
      await patchIssueFields(workingIssue.rawId, payload);
      currentIssue = { ...workingIssue };
      if (onIssueUpdated) {
        onIssueUpdated(workingIssue);
      }
      return true;
    } catch (err) {
      setStatus(modal, err.message || "Failed to save changes.", true);
      return false;
    }
  };

  const close = async (discard = false) => {
    const modal = backdrop.querySelector(".modal");
    if (modal && !discard) {
      const ok = await persistChanges(modal);
      if (!ok) {
        return;
      }
    }
    backdrop.classList.remove("open");
    backdrop.innerHTML = "";
    currentIssue = null;
    workingIssue = null;
  };

  const openDetail = (issue) => {
    backdrop.innerHTML = "";
    backdrop.appendChild(buildDetail(issue));
    backdrop.classList.add("open");
  };

  // Basic placeholders for create/edit until wired to real forms.
  const openCreate = () => {
    openDetail(emptyIssue);
  };

  const openEdit = (issue) => {
    openDetail(issue);
  };

  backdrop.addEventListener("click", (evt) => {
    if (evt.target === backdrop) {
      void close();
    }
  });

  // Keep `open` alias for backward compatibility.
  return { openDetail, openCreate, openEdit, close, open: openDetail };
};
