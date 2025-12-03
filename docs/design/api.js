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

const mapTags = (tags) =>
  (tags || []).map((t) => ({
    label: t.tag || "Tag",
    color: t.color || "#49a3d8"
  }));

export const setActiveDatabaseName = (name) => {
  activeDatabaseName = name || undefined;
};

export const getActiveDatabaseName = () => activeDatabaseName;

export const mapIssue = (dto, activeDatabase = activeDatabaseName) => {
  const rawIdStr = dto && dto.id !== undefined && dto.id !== null ? `${dto.id}` : "";
  const numericId = Number(rawIdStr.replace(/^#/, ""));
  const rawId = Number.isNaN(numericId) ? rawIdStr : numericId;
  const createdAtRaw = pick(dto, ["createdAt", "created_at"]);
  const authorRaw = pick(dto, ["author", "authorId", "author_id"], "Author");
  const statusRaw = pick(dto, ["status"], "Milestone");

  return {
    rawId,
    id: rawId !== undefined && rawId !== null && rawId !== "" ? `#${rawId}` : "#?",
    title: dto.title || "Untitled Issue",
    database: activeDatabase || dto.assignedTo || pick(dto, ["database", "db"], "Database name"),
    createdAt: fmtDate(createdAtRaw),
    author: authorRaw,
    milestone: statusRaw,
    status: statusRaw,
    description: dto.description || "",
    tags: mapTags(dto.tags),
    comments: []
  };
};

const mapComment = (dto) => ({
  author: dto.authorId || "Unknown",
  date: fmtDate(dto.timestamp),
  text: dto.text || ""
});

export const fetchIssues = async (activeDatabase) => {
  const path = "/issues";
  const res = await fetch(`${apiBase()}${path}`);
  const json = await handleResponse(res, path);
  const db = activeDatabase || activeDatabaseName;
  return (json || []).map((dto) => mapIssue(dto, db));
};

export const fetchComments = async (issueId) => {
  if (issueId === undefined || issueId === null) return [];
  const path = `/issues/${issueId}/comments`;
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

export const fetchUsers = async () => {
  const path = "/users";
  const res = await fetch(`${apiBase()}${path}`);
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

export const createIssue = async ({ title, description, authorId }) => {
  const path = "/issues";
  const res = await fetch(`${apiBase()}${path}`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    // Server DTO uses "author_id" as the JSON field name.
    body: JSON.stringify({
      title,
      description,
      authorId,      // keep for compatibility
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
  fetchComments,
  fetchActiveDatabase,
  fetchUsers,
  createUser,
  createIssue,
  patchIssueFields,
  deleteIssue
};
