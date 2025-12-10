const issues = [];

function renderIssues() {
  const list = document.getElementById("issues-list");
  list.innerHTML = "";

  if (issues.length === 0) {
    list.innerHTML =
      '<p style="color:#9ca3af;font-size:0.9rem;">No issues yet. Create one above.</p>';
    return;
  }

  issues.forEach((issue, index) => {
    const div = document.createElement("div");
    div.className = "issue-item";

    const header = document.createElement("div");
    header.className = "issue-header";

    const title = document.createElement("div");
    title.className = "issue-title";
    title.textContent = `#${index + 1} – ${issue.title}`;

    const status = document.createElement("div");
    status.className = `issue-status status-${issue.status}`;
    status.textContent = issue.status.replace("_", " ");

    header.appendChild(title);
    header.appendChild(status);

    const desc = document.createElement("p");
    desc.style.fontSize = "0.9rem";
    desc.style.marginTop = "0.3rem";
    desc.textContent = issue.description;

    const tags = document.createElement("div");
    tags.className = "issue-tags";
    tags.textContent =
      issue.tags.length > 0 ? `Tags: ${issue.tags.join(", ")}` : "No tags";

    div.appendChild(header);
    div.appendChild(desc);
    div.appendChild(tags);

    list.appendChild(div);
  });
}

document.getElementById("issue-form").addEventListener("submit", (e) => {
  e.preventDefault();

  const title = document.getElementById("title").value.trim();
  const description = document.getElementById("description").value.trim();
  const status = document.getElementById("status").value;
  const tagsRaw = document.getElementById("tags").value.trim();

  const tags =
    tagsRaw === ""
      ? []
      : tagsRaw.split(",").map((t) => t.trim()).filter((t) => t.length > 0);

  if (!title || !description) {
    alert("Please fill in title and description.");
    return;
  }

  issues.push({ title, description, status, tags });
  renderIssues();

  e.target.reset();
  document.getElementById("status").value = "open";
});

document.getElementById("year").textContent = new Date().getFullYear();

// initial render
renderIssues();
