import {
  issueCardTemplate,
  tagTemplate,
  actionButtonTemplate
} from "./templates.js";

export const renderIssues = ({ container, issues, onOpen }) => {
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
    actionsWrap.appendChild(actionButtonTemplate("Edit", "edit"));
    actionsWrap.appendChild(actionButtonTemplate("Delete", "delete"));

    fragment.appendChild(card);
  });

  container.replaceChildren(fragment);
};
