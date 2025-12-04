const DEFAULT_API_BASE = "http://localhost:8600";
let activeDatabaseName;

const apiBase = () => {
  if (window.API_BASE) return window.API_BASE;
  if (localStorage.getItem("API_BASE")) return localStorage.getItem("API_BASE");
  return DEFAULT_API_BASE;
};

const handleResponse = async (res, path) => {
  if (!res.ok) {
    const text = await res.text();
    throw new Error(`Request failed for ${path}: ${res.status} ${text}`);
  }
  return res.json();
};

const fmtDate = (value) => {
  if (value === undefined || value === null) return "Unknown date";
  if (typeof value === "string") {
    const trimmed = value.trim();
    if (!trimmed) return "Unknown date";
    const asNum = Number(trimmed);
    if (Number.isNaN(asNum)) return trimmed;
    value = asNum;
  }
  const num = Number(value);
  if (Number.isNaN(num)) return "Unknown date";
  // createdAt is stored as chrono time since epoch (seconds or ms).
  const date = new Date(num > 1e12 ? num : num * 1000);
  if (Number.isNaN(date.getTime())) return "Unknown date";
  return date.toLocaleDateString();
};

const pick = (obj, keys, fallback = undefined) => {
  for (const k of keys) {
    if (obj && obj[k] !== undefined && obj[k] !== null) {
      return obj[k];
    }
  }
  return fallback;
};

const STATUS_LABELS = {
  TODO: "To Be Done",
  IN_PROGRESS: "In Progress",
  DONE: "Done"
};

const normalizeStatusValue = (value) => {
  if (value === undefined || value === null) return "";
  const trimmed = `${value}`.trim();
  if (!trimmed) return "";

  const lower = trimmed.toLowerCase();
  if (trimmed === "1" || lower === "todo" || lower === "to be done") {
    return STATUS_LABELS.TODO;
  }
  if (trimmed === "2" || lower === "in progress") {
    return STATUS_LABELS.IN_PROGRESS;
  }
  if (trimmed === "3" || lower === "done") {
    return STATUS_LABELS.DONE;
  }
  return trimmed;
};

const mapTags = (tags) =>
  (tags || []).map((t) => ({
    label: t.tag || t.label || "Tag",
    color: t.color || "#49a3d8"
  }));

const mapComment = (dto = {}) => {
  const author = pick(dto, ["author", "authorId", "author_id"], "Unknown");
  const date = pick(dto, ["timestamp", "date"]);
  const idRaw = pick(dto, ["id", "commentId", "comment_id"]);
  return {
    id: idRaw,
    author: author || "Unknown",
    date: fmtDate(date),
    text: dto.text || dto.body || ""
  };
};

export const setActiveDatabaseName = (name) => {
  activeDatabaseName = name || undefined;
};

export const getActiveDatabaseName = () => activeDatabaseName;

export const mapIssue = (dto, activeDatabase = activeDatabaseName) => {
  const hasId = dto && dto.id !== undefined && dto.id !== null && dto.id !== "";
  let rawId;
  if (hasId) {
    const rawIdStr = `${dto.id}`.replace(/^#/, "");
    const numericId = Number(rawIdStr);
    rawId = Number.isNaN(numericId) ? rawIdStr : numericId;
  }
  const createdAtRaw = pick(dto, ["createdAt", "created_at"]);
  const authorRaw = pick(dto, ["author", "authorId", "author_id"], "Author");
  const statusRaw = pick(dto, ["status"], STATUS_LABELS.TODO);
  const status = normalizeStatusValue(statusRaw);
  const milestoneRaw = pick(
    dto,
    ["milestone", "milestoneName", "milestone_title"],
    ""
  );
  const assignedRaw = pick(dto, ["assignedTo", "assigned_to"]);
  const hasComments = Array.isArray(dto?.comments);
  const commentsRaw = hasComments ? dto.comments.map(mapComment) : undefined;

  return {
    rawId,
    id: rawId !== undefined && rawId !== null && rawId !== "" ? `#${rawId}` : "#?",
    title: dto.title || "Untitled Issue",
    database:
      activeDatabase || dto.assignedTo || pick(dto, ["database", "db"], "Database name"),
    createdAt: fmtDate(createdAtRaw),
    author: authorRaw,
    milestone: milestoneRaw || status,
    status,
    description: dto.description || "",
    assignedTo: assignedRaw || "",
    tags: mapTags(dto.tags),
    ...(hasComments ? { comments: commentsRaw } : {})
  };
};

export const fetchIssues = async (activeDatabase) => {
  const path = "/issues";
  const res = await fetch(`${apiBase()}${path}`);
  const json = await handleResponse(res, path);
  const db = activeDatabase || activeDatabaseName;
  return (json || []).map((dto) => mapIssue(dto, db));
};

export const fetchIssueById = async (issueId, activeDatabase) => {
  const id = normalizeIssueId(issueId);
  const path = `/issues/${id}`;
  const res = await fetch(`${apiBase()}${path}`);
  const json = await handleResponse(res, path);
  const db = activeDatabase || activeDatabaseName;
  return mapIssue(json, db);
};

export const fetchComments = async (issueId) => {
  if (issueId === undefined || issueId === null) return [];
  const path = `/issues/${normalizeIssueId(issueId)}/comments`;
  const res = await fetch(`${apiBase()}${path}`);
  const json = await handleResponse(res, path);
  return (json || []).map(mapComment);
};

export const fetchActiveDatabase = async () => {
  const path = "/databases";
  const res = await fetch(`${apiBase()}${path}`);
  const json = await handleResponse(res, path);
  if (!Array.isArray(json)) return undefined;
  const active = json.find((db) => db.active === true);
  const first = json.find((db) => db.name);
  const resolved = (active && active.name) || (first && first.name) || undefined;
  setActiveDatabaseName(resolved);
  return resolved;
};

export const setApiBase = (base) => {
  localStorage.setItem("API_BASE", base);
};

const normalizeIssueId = (issueId) => {
  if (issueId === undefined || issueId === null) {
    throw new Error("Missing issue id");
  }
  if (typeof issueId === "string") {
    const cleaned = issueId.startsWith("#") ? issueId.slice(1) : issueId;
    const asNum = Number(cleaned);
    if (!Number.isNaN(asNum)) return asNum;
    return cleaned;
  }
  return issueId;
};

const patchIssueField = async (issueId, field, value) => {
  const id = normalizeIssueId(issueId);
  const path = `/issues/${id}`;
  const res = await fetch(`${apiBase()}${path}`, {
    method: "PATCH",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ field, value })
  });
  if (!res.ok) {
    const text = await res.text();
    throw new Error(`Request failed for ${path}: ${res.status} ${text}`);
  }
};

export const patchIssueFields = async (issueId, updates = {}) => {
  const id = normalizeIssueId(issueId);
  const tasks = [];
  if (updates.title !== undefined) {
    tasks.push(patchIssueField(id, "title", updates.title));
  }
  if (updates.description !== undefined) {
    tasks.push(patchIssueField(id, "description", updates.description));
  }
  if (updates.status !== undefined) {
    tasks.push(patchIssueField(id, "status", updates.status));
  }

  for (const task of tasks) {
    await task;
  }
};

export const fetchIssueTags = async (issueId) => {
  const id = normalizeIssueId(issueId);
  const path = `/issues/${id}/tags`;
  const res = await fetch(`${apiBase()}${path}`);
  const json = await handleResponse(res, path);
  return mapTags(json);
};

export const fetchAllTags = async () => {
  const path = "/tags";
  const res = await fetch(`${apiBase()}${path}`);
  const json = await handleResponse(res, path);
  return mapTags(json);
};

const normalizeTagInput = (tagInput) => {
  if (typeof tagInput === "string") return tagInput.trim();
  if (tagInput && typeof tagInput === "object") {
    const candidate = tagInput.tag || tagInput.label || "";
    return candidate.trim();
  }
  return "";
};

export const addTagToIssue = async (issueId, tagInput = {}) => {
  const id = normalizeIssueId(issueId);
  const tag = normalizeTagInput(tagInput);
  const color = typeof tagInput === "object" ? (tagInput.color || "") : "";
  if (!tag) throw new Error("Tag name is required.");

  const payload = { tag };
  if (color) {
    payload.color = color;
  }

  const path = `/issues/${id}/tags`;
  const res = await fetch(`${apiBase()}${path}`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(payload)
  });
  if (!res.ok) {
    const text = await res.text();
    throw new Error(`Request failed for ${path}: ${res.status} ${text}`);
  }
  // Return the latest tags list to keep UI in sync.
  return fetchIssueTags(id);
};

export const removeTagFromIssue = async (issueId, tagInput) => {
  const id = normalizeIssueId(issueId);
  const tag = normalizeTagInput(tagInput);
  if (!tag) throw new Error("Tag name is required to remove.");

  const path = `/issues/${id}/tags`;
  const res = await fetch(`${apiBase()}${path}`, {
    method: "DELETE",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ tag })
  });
  if (!res.ok) {
    const text = await res.text();
    throw new Error(`Request failed for ${path}: ${res.status} ${text}`);
  }
};

export const createComment = async (issueId, { text, authorId }) => {
  const id = normalizeIssueId(issueId);
  const trimmed = (text || "").trim();
  const author = (authorId || "").toString().trim();
  if (!trimmed) throw new Error("Comment text is required.");
  if (!author) throw new Error("Author is required to add a comment.");

  const path = `/issues/${id}/comments`;
  const res = await fetch(`${apiBase()}${path}`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ text: trimmed, authorId: author, author_id: author })
  });
  const json = await handleResponse(res, path);
  return mapComment(json);
};

export const updateComment = async (issueId, commentId, text) => {
  const id = normalizeIssueId(issueId);
  const comment = normalizeIssueId(commentId);
  const trimmed = (text || "").trim();
  if (!trimmed) throw new Error("Comment text is required.");

  const path = `/issues/${id}/comments/${comment}`;
  const res = await fetch(`${apiBase()}${path}`, {
    method: "PATCH",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ text: trimmed })
  });
  if (!res.ok) {
    const body = await res.text();
    throw new Error(`Request failed for ${path}: ${res.status} ${body}`);
  }
  return trimmed;
};

export const deleteComment = async (issueId, commentId) => {
  const id = normalizeIssueId(issueId);
  const comment = normalizeIssueId(commentId);
  const path = `/issues/${id}/comments/${comment}`;
  const res = await fetch(`${apiBase()}${path}`, { method: "DELETE" });
  if (!res.ok) {
    const body = await res.text();
    throw new Error(`Request failed for ${path}: ${res.status} ${body}`);
  }
};

export const assignUserToIssue = async (issueId, userName) => {
  const id = normalizeIssueId(issueId);
  const user = (userName || "").toString().trim();
  if (!user) throw new Error("User is required to assign.");

  const path = `/users/${encodeURIComponent(user)}/issues`;
  const res = await fetch(`${apiBase()}${path}`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ issueId: id })
  });
  const json = await handleResponse(res, path);
  return mapIssue(json);
};

export const unassignIssue = async (issueId) => {
  const id = normalizeIssueId(issueId);
  const path = `/issues/${id}/unassign`;
  const res = await fetch(`${apiBase()}${path}`, { method: "PATCH" });
  const json = await handleResponse(res, path);
  return mapIssue(json);
};

export const fetchUsers = async () => {
  const path = "/users";
  const res = await fetch(`${apiBase()}${path}`);
  return handleResponse(res, path);
};

export const fetchUserRoles = async () => {
  const path = "/users/roles";
  const res = await fetch(`${apiBase()}${path}`);
  return handleResponse(res, path);
};

export const fetchDatabases = async () => {
  const path = "/databases";
  const res = await fetch(`${apiBase()}${path}`);
  return handleResponse(res, path);
};

export const createDatabase = async (name) => {
  if (!name) throw new Error("Database name is required.");
  const path = "/databases";
  const res = await fetch(`${apiBase()}${path}`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ name })
  });
  return handleResponse(res, path);
};

export const deleteDatabase = async (name) => {
  if (!name) throw new Error("Database name is required.");
  const encoded = encodeURIComponent(name);
  const path = `/databases/${encoded}`;
  const res = await fetch(`${apiBase()}${path}`, { method: "DELETE" });
  if (!res.ok) {
    const text = await res.text();
    throw new Error(`Request failed for ${path}: ${res.status} ${text}`);
  }
};

export const switchDatabase = async (name) => {
  if (!name) throw new Error("Database name is required.");
  const encoded = encodeURIComponent(name);
  const path = `/databases/${encoded}/switch`;
  const res = await fetch(`${apiBase()}${path}`, { method: "POST" });
  return handleResponse(res, path);
};

export const renameDatabase = async (currentName, newName) => {
  if (!currentName || !newName) {
    throw new Error("Current and new database names are required.");
  }
  const encoded = encodeURIComponent(currentName);
  const path = `/databases/${encoded}`;
  const res = await fetch(`${apiBase()}${path}`, {
    method: "PATCH",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ name: newName })
  });
  return handleResponse(res, path);
};

export const createUser = async ({ name, role }) => {
  const path = "/users";
  const res = await fetch(`${apiBase()}${path}`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ name, role })
  });
  return handleResponse(res, path);
};

export const updateUser = async (id, updates = {}) => {
  if (!id) throw new Error("User id is required to update.");
  const tasks = [];
  const path = (field) => `/users/${encodeURIComponent(id)}`;
  const send = async (field, value) => {
    const res = await fetch(`${apiBase()}${path(field)}`, {
      method: "PATCH",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ field, value })
    });
    if (!res.ok) {
      const text = await res.text();
      throw new Error(`Request failed for ${path(field)}: ${res.status} ${text}`);
    }
  };
  if (updates.name !== undefined) {
    tasks.push(send("name", updates.name));
  }
  if (updates.role !== undefined) {
    tasks.push(send("role", updates.role));
  }
  for (const task of tasks) {
    await task;
  }
};

export const deleteUser = async (id) => {
  if (!id) throw new Error("User id is required to delete.");
  const path = `/users/${encodeURIComponent(id)}`;
  const res = await fetch(`${apiBase()}${path}`, { method: "DELETE" });
  if (!res.ok) {
    const text = await res.text();
    throw new Error(`Request failed for ${path}: ${res.status} ${text}`);
  }
};

export const createIssue = async ({ title, description, authorId }) => {
  const path = "/issues";
  const res = await fetch(`${apiBase()}${path}`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    // Server DTO uses "author_id" as the JSON field name.
    body: JSON.stringify({
      title,
      description,
      authorId, // keep for compatibility
      author_id: authorId
    })
  });
  const created = await handleResponse(res, path);
  return mapIssue(created);
};

export const deleteIssue = async (issueId) => {
  const id = normalizeIssueId(issueId);
  const path = `/issues/${id}`;
  const res = await fetch(`${apiBase()}${path}`, {
    method: "DELETE"
  });
  if (!res.ok) {
    const text = await res.text();
    throw new Error(`Request failed for ${path}: ${res.status} ${text}`);
  }
};

export const apiClient = {
  setActiveDatabaseName,
  getActiveDatabaseName,
  mapIssue,
  fetchIssues,
  fetchIssueById,
  fetchComments,
  createComment,
  updateComment,
  deleteComment,
  assignUserToIssue,
  unassignIssue,
  fetchActiveDatabase,
  fetchDatabases,
  fetchUsers,
  fetchUserRoles,
  createUser,
  updateUser,
  deleteUser,
  createDatabase,
  deleteDatabase,
  switchDatabase,
  renameDatabase,
  createIssue,
  patchIssueFields,
  deleteIssue,
  fetchIssueTags,
  fetchAllTags,
  addTagToIssue,
  removeTagFromIssue
};
