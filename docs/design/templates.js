const htmlToElement = (html) => {
  const tpl = document.createElement("template");
  tpl.innerHTML = html.trim();
  return tpl.content.firstElementChild;
};

export const issueCardTemplate = () =>
  htmlToElement(`
    <article class="issue-card">
      <h2 class="issue-title">
        <button class="issue-title-btn" data-role="title"></button>
      </h2>
      <div class="issue-meta">
        <span data-field="database"></span>
        <span data-field="createdAt"></span>
        <span data-field="author"></span>
        <span data-field="milestone"></span>
      </div>
      <div class="card-actions">
        <div class="tags" data-role="tags"></div>
        <div class="actions-right" data-role="actions"></div>
      </div>
    </article>
  `);

export const tagTemplate = (tag) => {
  const el = htmlToElement(`<span class="tag"></span>`);
  el.textContent = tag.label;
  el.style.backgroundColor = tag.color;
  return el;
};

export const actionButtonTemplate = (label, variant) => {
  const el = htmlToElement(`<button type="button" class="action-btn"></button>`);
  el.textContent = label;
  if (variant) {
    el.classList.add(variant);
  }
  return el;
};

export const modalBackdropTemplate = () =>
  htmlToElement(`<div class="modal-backdrop"></div>`);

export const modalTemplate = () =>
  htmlToElement(`
    <div class="modal">
      <div class="modal-header">
        <span data-field="heading"></span>
        <button type="button" class="modal-close">X</button>
      </div>
      <div class="detail-grid">
        <div>
          <div class="detail-title-box" data-field="titleBox"></div>
          <div class="detail-description" data-field="description"></div>
        </div>
        <div class="detail-sidebar" data-role="sidebar"></div>
      </div>
      <button type="button" class="add-comment">Add Comment</button>
      <div class="comments" data-role="comments"></div>
    </div>
  `);

export const pillTemplate = (label, className) => {
  const el = htmlToElement(`<button type="button" class="detail-pill"></button>`);
  el.textContent = label;
  if (className) {
    el.classList.add(className);
  }
  return el;
};

export const commentCardTemplate = (comment) => {
  const el = htmlToElement(`
    <div class="comment-card">
      <div class="comment-header">
        <span class="comment-meta"></span>
        <button type="button" class="comment-delete">Delete</button>
      </div>
      <div class="comment-body"></div>
    </div>
  `);
  el.querySelector(".comment-meta").textContent =
    `${comment.author} · ${comment.date}`;
  el.querySelector(".comment-body").textContent = comment.text;
  return el;
};
