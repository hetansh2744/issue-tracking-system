import {
  modalBackdropTemplate,
  modalTemplate,
  pillTemplate,
  commentCardTemplate,
  commentFormTemplate
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
  assignedTo: "",
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

  const startCommentInlineEdit = (modal, bodyEl, comment, index) => {
    if (!bodyEl || !comment) return;
    const original = comment.text || "";

    bodyEl.contentEditable = "true";
    bodyEl.classList.add("editing-inline");
    setStatus(modal, "Editing comment. Ctrl+Enter to save, Esc to cancel.");

    const selectAll = () => {
      const range = document.createRange();
      range.selectNodeContents(bodyEl);
      const sel = window.getSelection();
      sel.removeAllRanges();
      sel.addRange(range);
    };
    selectAll();

    const cleanup = () => {
      bodyEl.contentEditable = "false";
      bodyEl.classList.remove("editing-inline");
      bodyEl.removeEventListener("keydown", onKeyDown);
      bodyEl.removeEventListener("blur", onBlur);
    };

    const cancel = () => {
      bodyEl.textContent = original;
      cleanup();
      setStatus(modal, "");
    };

    const save = async () => {
      const next = bodyEl.textContent.trim();
      if (next.length === 0) {
        setStatus(modal, "Comment cannot be empty.", true);
        bodyEl.focus();
        return;
      }
      if (next === original) {
        cancel();
        return;
      }

      const addBtn = modal.querySelector(".add-comment");
      if (addBtn) addBtn.disabled = true;

      try {
        setStatus(modal, "Saving comment...");
        if (comment.id !== undefined && comment.id !== null && workingIssue?.rawId !== undefined && workingIssue?.rawId !== null) {
          await apiClient.updateComment(workingIssue.rawId, comment.id, next);
        }
        const nextComments = (workingIssue.comments || []).map((c, i) => {
          const matchesById =
            c.id !== undefined && c.id !== null &&
            comment.id !== undefined && comment.id !== null &&
            c.id === comment.id;
          if (matchesById || i === index) {
            return { ...c, text: next };
          }
          return c;
        });
        workingIssue = { ...workingIssue, comments: nextComments };
        currentIssue = { ...workingIssue };
        renderComments(modal, workingIssue);
        setStatus(modal, "Comment saved.");
        if (onIssueUpdated) {
          onIssueUpdated(workingIssue);
        }
      } catch (err) {
        setStatus(modal, err.message || "Failed to save comment.", true);
      } finally {
        cleanup();
        if (addBtn) addBtn.disabled = false;
      }
    };

    const onKeyDown = (evt) => {
      if (evt.key === "Escape") {
        evt.preventDefault();
        cancel();
      } else if (evt.key === "Enter" && (evt.ctrlKey || evt.metaKey)) {
        evt.preventDefault();
        bodyEl.blur();
      }
    };

    const onBlur = () => {
      void save();
    };

    bodyEl.addEventListener("keydown", onKeyDown);
    bodyEl.addEventListener("blur", onBlur, { once: true });
  };

  const handleDeleteComment = async (modal, comment, index) => {
    if (!workingIssue || workingIssue.rawId === undefined || workingIssue.rawId === null) {
      setStatus(modal, "Save the issue before deleting comments.", true);
      return;
    }

    const addBtn = modal.querySelector(".add-comment");
    if (addBtn) addBtn.disabled = true;

    try {
      if (comment.id !== undefined && comment.id !== null) {
        setStatus(modal, "Deleting comment...");
        await apiClient.deleteComment(workingIssue.rawId, comment.id);
      } else {
        setStatus(modal, "Removing comment locally (no id).");
      }
      const nextComments = (workingIssue.comments || []).filter((c, i) => {
        const matchesById =
          c.id !== undefined && c.id !== null &&
          comment.id !== undefined && comment.id !== null &&
          c.id === comment.id;
        return !(matchesById || i === index);
      });
      workingIssue = { ...workingIssue, comments: nextComments };
      currentIssue = { ...workingIssue };
      renderComments(modal, workingIssue);
      setStatus(modal, "Comment deleted.");
      if (onIssueUpdated) {
        onIssueUpdated(workingIssue);
      }
    } catch (err) {
      setStatus(modal, err.message || "Failed to delete comment.", true);
    } finally {
      if (addBtn) addBtn.disabled = false;
    }
  };

  const renderComments = (modal, issue) => {
    const commentsWrap = modal.querySelector('[data-role="comments"]');
    if (!commentsWrap) return;
    commentsWrap.replaceChildren();
    (issue.comments || []).forEach((comment, idx) => {
      const card = commentCardTemplate(comment);
      const body = card.querySelector(".comment-body");
      const deleteBtn = card.querySelector(".comment-delete");
      body?.addEventListener("dblclick", () => {
        startCommentInlineEdit(modal, body, comment, idx);
      });
      deleteBtn?.addEventListener("click", (evt) => {
        evt.preventDefault();
        void handleDeleteComment(modal, comment, idx);
      });
      commentsWrap.appendChild(card);
    });
  };

  const loadUsers = async () => {
    if (usersCache) return usersCache;
    const users = await fetchUsers();
    usersCache = users;
    return users;
  };

  const normalizeUser = (u) => (u && u.name) || u?.id || u || "";

  const findUserByName = (users, name) => {
    if (!name) return null;
    const target = name.toLowerCase();
    return (users || []).find((u) => normalizeUser(u).toLowerCase() === target) || null;
  };

  const startAssigneeEdit = async (modal, pillEl) => {
    if (!workingIssue || workingIssue.rawId === undefined || workingIssue.rawId === null) {
      setStatus(modal, "Save the issue before assigning.", true);
      return;
    }

    const originalText = pillEl.textContent;
    pillEl.classList.add("editing-assignee");
    pillEl.textContent = "";
    const input = document.createElement("input");
    input.type = "text";
    input.placeholder = "Enter user to assign";
    input.value = workingIssue.assignedTo || "";
    input.classList.add("assignee-input");
    pillEl.appendChild(input);
    input.focus();
    input.select();

    const restoreLabel = () => {
      pillEl.classList.remove("editing-assignee");
      pillEl.textContent = originalText;
    };

    const applyLabel = (assignee) => {
      pillEl.classList.remove("editing-assignee");
      pillEl.textContent = assignee ? `Assignees: ${assignee}` : "Assignees";
    };

    const onCancel = () => {
      restoreLabel();
      setStatus(modal, "");
    };

    const commit = async () => {
      const value = (input.value || "").trim();
      let users = [];
      if (value) {
        try {
          users = await loadUsers();
        } catch (err) {
          setStatus(modal, err.message || "Failed to load users.", true);
          input.focus();
          input.select();
          return;
        }
      }
      const match = findUserByName(users, value);

      if (!value) {
        try {
          setStatus(modal, "Unassigning...");
          const updated = await apiClient.unassignIssue(workingIssue.rawId);
          const dbName =
            (getActiveDatabase && getActiveDatabase()) || apiClient.getActiveDatabaseName();
        const mapped = mapIssue(updated, dbName);
        workingIssue = { ...workingIssue, ...mapped };
        currentIssue = { ...workingIssue };
        fillView(modal, workingIssue);
        setStatus(modal, "Issue unassigned.");
        if (onIssueUpdated) {
          onIssueUpdated(workingIssue);
        }
      } catch (err) {
        setStatus(modal, err.message || "Failed to unassign.", true);
        restoreLabel();
      }
      return;
      }

      if (!match) {
        setStatus(modal, "User not found. Try another name.", true);
        input.focus();
        input.select();
        return;
      }

      if (value === workingIssue.assignedTo) {
        applyLabel(value);
        setStatus(modal, "");
        return;
      }

      try {
        setStatus(modal, "Assigning user...");
        const updated = await apiClient.assignUserToIssue(workingIssue.rawId, value);
        const dbName =
          (getActiveDatabase && getActiveDatabase()) || apiClient.getActiveDatabaseName();
        const mapped = mapIssue(updated, dbName);
        workingIssue = { ...workingIssue, ...mapped };
        currentIssue = { ...workingIssue };
        fillView(modal, workingIssue);
        setStatus(modal, `Assignee set to ${workingIssue.assignedTo || value}.`);
        if (onIssueUpdated) {
          onIssueUpdated(workingIssue);
        }
      } catch (err) {
        setStatus(modal, err.message || "Failed to assign user.", true);
        restoreLabel();
      }
    };

    const onKeyDown = (evt) => {
      if (evt.key === "Escape") {
        evt.preventDefault();
        onCancel();
      } else if (evt.key === "Enter") {
        evt.preventDefault();
        void commit();
      }
    };

    const onBlur = () => {
      void commit();
    };

    input.addEventListener("keydown", onKeyDown);
    input.addEventListener("blur", onBlur, { once: true });
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
    const assigneePill = sidebar.querySelector(".detail-pill.assignees");
    assigneePill?.addEventListener("dblclick", () => {
      void startAssigneeEdit(modal, assigneePill);
    });
    renderComments(modal, issue);
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
    loadUsers()
      .then((users) => {
        populateAuthor(users);
      })
      .catch((err) => {
        authorWrap.textContent = err.message || "Failed to load users.";
        authorWrap.classList.add("error");
      });
  };

  const openCommentForm = async (modal) => {
    if (!workingIssue || workingIssue.rawId === undefined || workingIssue.rawId === null) {
      setStatus(modal, "Save the issue before adding comments.", true);
      return;
    }

    const existing = modal.querySelector('[data-role="comment-form"]');
    if (existing) {
      existing.querySelector('[data-field="comment-text"]')?.focus();
      return;
    }

    const form = commentFormTemplate();
    const authorSelect = form.querySelector('[data-field="comment-author"]');
    const textArea = form.querySelector('[data-field="comment-text"]');
    const cancelBtn = form.querySelector('[data-role="cancel-comment"]');
    const region = modal.querySelector('[data-role="comment-form-region"]');

    const populateAuthors = (users) => {
      if (!authorSelect) return;
      authorSelect.innerHTML = "";

      if (!Array.isArray(users) || users.length === 0) {
        const fallback = workingIssue.author || workingIssue.authorId || "Author";
        const opt = document.createElement("option");
        opt.value = fallback;
        opt.textContent = fallback;
        authorSelect.appendChild(opt);
        return fallback;
      }

      users.forEach((u) => {
        const name = u.name || u.id || u;
        const opt = document.createElement("option");
        opt.value = name;
        opt.textContent = name;
        authorSelect.appendChild(opt);
      });

      const preferred = workingIssue.author || workingIssue.authorId;
      const defaultAuthor = users[0].name || users[0].id || users[0];
      const resolved = preferred || defaultAuthor;
      authorSelect.value = resolved;
      return resolved;
    };

    populateAuthors(usersCache || [workingIssue.author || workingIssue.authorId || "Author"]);
    loadUsers()
      .then((users) => {
        populateAuthors(users);
      })
      .catch((err) => {
        populateAuthors([]);
        setStatus(modal, err.message || "Failed to load users; using fallback author.", true);
      });

    const onSubmit = async (evt) => {
      evt.preventDefault();
      const text = (textArea?.value || "").trim();
      const authorId = (authorSelect?.value || "").trim();
      if (!text) {
        setStatus(modal, "Comment cannot be empty.", true);
        textArea?.focus();
        return;
      }
      if (!authorId) {
        setStatus(modal, "Author is required for comments.", true);
        authorSelect?.focus();
        return;
      }

      const addBtn = modal.querySelector(".add-comment");
      if (addBtn) addBtn.disabled = true;

      try {
        setStatus(modal, "Posting comment...");
        const comment = await apiClient.createComment(workingIssue.rawId, { text, authorId });
        const nextComments = [...(workingIssue.comments || []), comment];
        workingIssue = { ...workingIssue, comments: nextComments };
        currentIssue = { ...workingIssue };
        renderComments(modal, workingIssue);
        if (region) {
          region.replaceChildren();
        } else {
          form.remove();
        }
        setStatus(modal, "Comment added.");
        if (onIssueUpdated) {
          onIssueUpdated(workingIssue);
        }
      } catch (err) {
        setStatus(modal, err.message || "Failed to add comment.", true);
      } finally {
        if (addBtn) addBtn.disabled = false;
      }
    };

    form.addEventListener("submit", onSubmit);
    cancelBtn?.addEventListener("click", (evt) => {
      evt.preventDefault();
      if (region) {
        region.replaceChildren();
      } else {
        form.remove();
      }
      setStatus(modal, "");
    });

    if (region) {
      region.replaceChildren(form);
    } else {
      modal.appendChild(form);
    }
    textArea?.focus();
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
    modal.querySelector(".add-comment")?.addEventListener("click", () => {
      void openCommentForm(modal);
    });
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
