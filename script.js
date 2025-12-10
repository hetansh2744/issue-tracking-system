// Frontend issue tracker talking to a real backend API.
// Adjust this to your deployed backend URL:
const API_BASE = "https://your-its-backend.onrender.com"; // TODO: change this

let issues = [];
let isLoading = false;

// Helpers
function formatDate(iso) {
  if (!iso) return "";
  const d = new Date(iso);
  return d.toLocaleString();
}

function setLoading(loading) {
  isLoading = loading;
  const list = document.getElementById("issues-list");
  if (loading) {
    list.innerHTML =
      '<div class="empty-state">Loading issues from server...</div>';
  }
}

function showError(msg) {
  alert(msg);
}

// API calls
async function fetchIssuesFromApi() {
  setLoading(true);
  try {
    const res = await fetch(`${API_BASE}/issues`);
    if (!res.ok) throw new Error(`Failed to load issues: ${res.status}`);
    const data = await res.json();
    // Expect data to be an array of issue objects
    issues = data;
  } catch (e) {
    console.error(e);
    showError("Could not load issues from server.");
  } finally {
    setLoading(false);
    renderIssues();
  }
}

async function createIssueApi(payload) {
  const res = await fetch(`${API_BASE}/issues`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(payload),
  });
  if (!res.ok) {
    throw new Error(`Failed to create issue: ${res.status}`);
  }
  return res.json(); // created issue
}

async function updateIssueApi(id, payload) {
  const res = await fetch(`${API_BASE}/issues/${id}`, {
    method: "PUT",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(payload),
  });
  if (!res.ok) {
    throw new Error(`Failed to update issue: ${res.status}`);
  }
  return res.json(); // updated issue
}

async function deleteIssueApi(id) {
  const res = await fetch(`${API_BASE}/issues/${id}`, {
    method: "DELETE",
  });
  if (!res.ok) {
    throw new Error(`Failed to delete issue: ${res.status}`);
  }
}

// UI updates
function updateStats() {
  const statsBar = document.getElementById("stats-bar");
  const total = issues.length;
  const openCount = issues.filter((i) => i.status === "open").length;
  const inProgCount = issues.filter((i) => i.status === "in_progress").length;
  const resCount = issues.filter((i) => i.status === "resolved").length;
  const closedCount = issues.filter((i) => i.status === "closed").length;

  statsBar.innerHTML = `
    <div class="stat-pill">Total: ${total}</div>
    <div class="stat-pill">Open: ${openCount}</div>
    <div class="stat-pill">In Progress: ${inProgCount}</div>
    <div class="stat-pill">Resolved: ${resCount}</div>
    <div class="stat-pill">Closed: ${closedCount}</div>
  `;
}

function applyFilters(issue, searchTerm, statusFilter) {
  if (statusFilter !== "all" && issue.status !== statusFilter) return false;

  if (searchTerm) {
    const s = searchTerm.toLowerCase();
    const haystack = [
      issue.title,
      issue.description,
      issue.assignee || "",
      (issue.tags || []).join(", "),
    ]
      .join(" ")
      .toLowerCase();
    if (!haystack.includes(s)) return false;
  }

  return true;
}

function renderIssues() {
  const list = document.getElementById("issues-list");
  if (isLoading) return; // already showing loading state

  const searchInput = document.getElementById("search");
  const statusFilter = document.getElementById("filter-status");
  const searchTerm = searchInput.value.trim().toLowerCase();
  const filterStatus = statusFilter.value;

  const filtered = issues.filter((issue) =>
    applyFilters(issue, searchTerm, filterStatus)
  );

  list.innerHTML = "";

  if (filtered.length === 0) {
    list.innerHTML =
      '<div class="empty-state">No issues found. Create one on the left or adjust your filters.</div>';
    updateStats();
    return;
  }

  filtered
    .slice()
    .sort((a, b) => b.id - a.id)
    .forEach((issue) => {
      const item = document.createElement("div");
      item.className = "issue-item";
      item.dataset.id = issue.id;

      const tagsText =
        issue.tags && issue.tags.length > 0
          ? issue.tags.join(", ")
          : "No tags";

      const assigneeText = issue.assignee
        ? `Assignee: ${issue.assignee}`
        : "Unassigned";

      item.innerHTML = `
        <div class="issue-header">
          <div>
            <div class="issue-title">${issue.title}</div>
            <div class="issue-id">#${issue.id}</div>
          </div>
          <div class="issue-status status-${issue.status}">
            ${issue.status.replace("_", " ")}
          </div>
        </div>

        <div class="issue-desc">
          ${issue.description}
        </div>

        <div class="issue-meta">
          <div>${assigneeText}</div>
          <div class="issue-tags"><span>Tags:</span> ${tagsText}</div>
        </div>

        <div class="issue-footer">
          <div class="issue-dates">
            <div>Created: ${formatDate(issue.createdAt)}</div>
            <div>Updated: ${formatDate(issue.updatedAt)}</div>
          </div>
          <div class="issue-actions">
            <button class="secondary js-edit">Edit</button>
            <button class="secondary js-cycle-status">Next status</button>
            <button class="danger js-delete">Delete</button>
          </div>
        </div>
      `;

      list.appendChild(item);
    });

  updateStats();
}

function resetForm() {
  const form = document.getElementById("issue-form");
  form.reset();
  document.getElementById("issue-id").value = "";
  document.getElementById("status").value = "open";
  document.getElementById("form-title").textContent = "Create New Issue";
  document.getElementById("save-btn").textContent = "Create Issue";
  document.getElementById("cancel-edit-btn").style.display = "none";
}

// Create / update handler
async function handleFormSubmit(e) {
  e.preventDefault();

  const idField = document.getElementById("issue-id");
  const titleField = document.getElementById("title");
  const descField = document.getElementById("description");
  const assigneeField = document.getElementById("assignee");
  const statusField = document.getElementById("status");
  const tagsField = document.getElementById("tags");

  const title = titleField.value.trim();
  const description = descField.value.trim();
  const assignee = assigneeField.value.trim();
  const status = statusField.value;
  const tagsRaw = tagsField.value.trim();

  if (!title || !description) {
    showError("Title and description are required.");
    return;
  }

  const tags =
    tagsRaw === ""
      ? []
      : tagsRaw
          .split(",")
          .map((t) => t.trim())
          .filter((t) => t.length > 0);

  const existingId = idField.value ? parseInt(idField.value, 10) : null;

  const payload = {
    title,
    description,
    assignee: assignee || "",
    status,
    tags,
  };

  try {
    let updatedOrNew;
    if (existingId) {
      updatedOrNew = await updateIssueApi(existingId, payload);
      const idx = issues.findIndex((i) => i.id === existingId);
      if (idx !== -1) issues[idx] = updatedOrNew;
    } else {
      updatedOrNew = await createIssueApi(payload);
      issues.push(updatedOrNew);
    }
    renderIssues();
    resetForm();
  } catch (e) {
    console.error(e);
    showError("Failed to save issue on server.");
  }
}

function beginEditIssue(id) {
  const issue = issues.find((i) => i.id === id);
  if (!issue) return;

  document.getElementById("issue-id").value = issue.id;
  document.getElementById("title").value = issue.title;
  document.getElementById("description").value = issue.description;
  document.getElementById("assignee").value = issue.assignee || "";
  document.getElementById("status").value = issue.status;
  document.getElementById("tags").value = (issue.tags || []).join(", ");

  document.getElementById("form-title").textContent = `Edit Issue #${issue.id}`;
  document.getElementById("save-btn").textContent = "Save Changes";
  document.getElementById("cancel-edit-btn").style.display = "inline-flex";

  window.scrollTo({ top: 0, behavior: "smooth" });
}

async function deleteIssue(id) {
  if (!confirm(`Are you sure you want to delete issue #${id}?`)) return;
  try {
    await deleteIssueApi(id);
    issues = issues.filter((i) => i.id !== id);
    renderIssues();
  } catch (e) {
    console.error(e);
    showError("Failed to delete issue on server.");
  }
}

async function cycleStatus(id) {
  const issue = issues.find((i) => i.id === id);
  if (!issue) return;

  const order = ["open", "in_progress", "resolved", "closed"];
  const currentIndex = order.indexOf(issue.status);
  const nextStatus = order[(currentIndex + 1) % order.length];

  const payload = {
    title: issue.title,
    description: issue.description,
    assignee: issue.assignee || "",
    status: nextStatus,
    tags: issue.tags || [],
  };

  try {
    const updated = await updateIssueApi(id, payload);
    const idx = issues.findIndex((i) => i.id === id);
    if (idx !== -1) issues[idx] = updated;
    renderIssues();
  } catch (e) {
    console.error(e);
    showError("Failed to update status on server.");
  }
}

// Clear-all button can call a backend batch delete endpoint if you add one
// For now, we just loop DELETE (optional feature).

async function clearAllIssues() {
  if (!issues.length) {
    showError("There are no issues to clear.");
    return;
  }
  if (!confirm("Clear ALL issues from server? This cannot be undone.")) {
    return;
  }

  try {
    // naive: delete one by one
    for (const issue of issues) {
      await deleteIssueApi(issue.id);
    }
    issues = [];
    renderIssues();
  } catch (e) {
    console.error(e);
    showError("Failed to clear all issues on server.");
  }
}

// Wire up events
document.addEventListener("DOMContentLoaded", () => {
  document.getElementById("year").textContent = new Date().getFullYear();

  document
    .getElementById("issue-form")
    .addEventListener("submit", handleFormSubmit);

  document
    .getElementById("cancel-edit-btn")
    .addEventListener("click", () => resetForm());

  document.getElementById("search").addEventListener("input", renderIssues);
  document
    .getElementById("filter-status")
    .addEventListener("change", renderIssues);

  document
    .getElementById("clear-all-btn")
    .addEventListener("click", clearAllIssues);

  document
    .getElementById("issues-list")
    .addEventListener("click", (event) => {
      const item = event.target.closest(".issue-item");
      if (!item) return;
      const id = parseInt(item.dataset.id, 10);

      if (event.target.classList.contains("js-edit")) {
        beginEditIssue(id);
      } else if (event.target.classList.contains("js-delete")) {
        deleteIssue(id);
      } else if (event.target.classList.contains("js-cycle-status")) {
        cycleStatus(id);
      }
    });

  // Initial load from backend
  fetchIssuesFromApi();
});
