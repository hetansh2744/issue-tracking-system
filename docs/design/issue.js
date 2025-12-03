import {
  issueCardTemplate,
  tagTemplate,
  actionButtonTemplate
} from "./templates.js";

export const renderIssues = ({ container, issues, onOpen, onEdit, onDelete }) => {
  const fragment = document.createDocumentFragment();

  issues.forEach((issue) => {
    const card = issueCardTemplate();

    const titleBtn = card.querySelector('[data-role="title"]');
    titleBtn.textContent = `${issue.title} (${issue.id})`;
    titleBtn.addEventListener("click", () => onOpen(issue));

    card.querySelector('[data-field="database"]').textContent = issue.database;
    card.querySelector('[data-field="createdAt"]').textContent = issue.createdAt;
    card.querySelector('[data-field="author"]').textContent = issue.author;
    card.querySelector('[data-field="milestone"]').textContent = issue.milestone;

    const tagsWrap = card.querySelector('[data-role="tags"]');
    issue.tags.forEach((tag) => tagsWrap.appendChild(tagTemplate(tag)));

    const actionsWrap = card.querySelector('[data-role="actions"]');
    const editBtn = actionButtonTemplate("Edit", "edit");
    editBtn.addEventListener("click", (evt) => {
      evt.stopPropagation();
      onEdit && onEdit(issue);
    });
    actionsWrap.appendChild(editBtn);

    const deleteBtn = actionButtonTemplate("Delete", "delete");
    deleteBtn.addEventListener("click", (evt) => {
      evt.stopPropagation();
      onDelete && onDelete(issue);
    });
    actionsWrap.appendChild(deleteBtn);

    // Open detail when clicking anywhere on the card except buttons.
    card.addEventListener("click", (evt) => {
      if (evt.target.tagName === "BUTTON") return;
      onOpen(issue);
    });

    fragment.appendChild(card);
  });

  container.replaceChildren(fragment);
};
