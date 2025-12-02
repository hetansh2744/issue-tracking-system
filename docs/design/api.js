const DEFAULT_API_BASE = "http://localhost:8600";

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

const mapIssue = (dto, activeDatabase) => {
  const createdAtRaw = pick(dto, ["createdAt", "created_at"]);
  const authorRaw = pick(dto, ["authorId", "author_id"], "Author");
  const statusRaw = pick(dto, ["status"], "Milestone");

  return {
    rawId: dto.id,
    id: dto.id !== undefined && dto.id !== null ? `#${dto.id}` : "#?",
    title: dto.title || "Untitled Issue",
    database: activeDatabase || dto.assignedTo || pick(dto, ["database", "db"], "Database name"),
    createdAt: fmtDate(createdAtRaw),
    author: authorRaw,
    milestone: statusRaw,
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
  return (json || []).map((dto) => mapIssue(dto, activeDatabase));
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
  if (active && active.name) return active.name;
  const first = json.find((db) => db.name);
  return first ? first.name : undefined;
};

export const setApiBase = (base) => {
  localStorage.setItem("API_BASE", base);
};
