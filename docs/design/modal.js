import {
  modalBackdropTemplate,
  modalTemplate,
  pillTemplate,
  commentCardTemplate
} from "./templates.js";
import { patchIssueFields, fetchUsers, createIssue, createUser, mapIssue } from "./api.js";

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
  let cachedDefaultAuthor = null;
  let currentIssue = null;
  let workingIssue = null;

  const resolveDefaultAuthor = async () => {
    if (cachedDefaultAuthor) return cachedDefaultAuthor;
    const users = await fetchUsers();
    if (Array.isArray(users) && users.length > 0) {
      // TODO: let the user choose an author instead of defaulting to the first.
      cachedDefaultAuthor = users[0].name || users[0].id || users[0];
      return cachedDefaultAuthor;
    }

    const generated = `default-user-${Date.now()}`;
    const createdUser = await createUser({
      name: generated,
      role: "Developer"
    });
    cachedDefaultAuthor = createdUser.name || generated;
    return cachedDefaultAuthor;
  };

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
  };

  const buildDetail = (issue) => {
    const modal = modalTemplate();
    modal.dataset.issueId = issue.rawId ?? "";
    currentIssue = { ...issue };
    workingIssue = { ...issue };
    fillView(modal, workingIssue);
    bindEditHandlers(modal);
    modal.querySelector(".modal-close").addEventListener("click", () => {
      void close();
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
      try {
        setStatus(modal, "Creating issue...");
        const authorId = await resolveDefaultAuthor();
        if (!authorId) {
          setStatus(modal, "No author available; please create a user.", true);
          return false;
        }
        const created = await createIssue({
          title,
          description: workingIssue.description || "",
          authorId
        });
        const mapped = mapIssue(
          created,
          getActiveDatabase ? getActiveDatabase() : undefined
        );
        workingIssue = { ...workingIssue, ...mapped };
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

  const close = async () => {
    const modal = backdrop.querySelector(".modal");
    if (modal) {
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
