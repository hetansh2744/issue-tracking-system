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
  const date = new Date(Number(value));
  if (Number.isNaN(date.getTime())) return "Unknown date";
  return date.toLocaleDateString();
};

const mapTags = (tags) =>
  (tags || []).map((t) => ({
    label: t.tag || "Tag",
    color: t.color || "#49a3d8"
  }));

const mapIssue = (dto) => ({
  rawId: dto.id,
  id: dto.id !== undefined && dto.id !== null ? `#${dto.id}` : "#?",
  title: dto.title || "Untitled Issue",
  database: dto.assignedTo || "Unassigned",
  createdAt: fmtDate(dto.createdAt),
  author: dto.authorId || "Unknown",
  milestone: dto.status || "No status",
  description: dto.description || "",
  tags: mapTags(dto.tags),
  comments: []
});

const mapComment = (dto) => ({
  author: dto.authorId || "Unknown",
  date: fmtDate(dto.timestamp),
  text: dto.text || ""
});

export const fetchIssues = async () => {
  const path = "/issues";
  const res = await fetch(`${apiBase()}${path}`);
  const json = await handleResponse(res, path);
  return (json || []).map(mapIssue);
};

export const fetchComments = async (issueId) => {
  if (issueId === undefined || issueId === null) return [];
  const path = `/issues/${issueId}/comments`;
  const res = await fetch(`${apiBase()}${path}`);
  const json = await handleResponse(res, path);
  return (json || []).map(mapComment);
};

export const setApiBase = (base) => {
  localStorage.setItem("API_BASE", base);
};
