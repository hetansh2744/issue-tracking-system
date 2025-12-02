import {
  modalBackdropTemplate,
  modalTemplate,
  pillTemplate,
  commentCardTemplate
} from "./templates.js";

const pillConfig = [
  { label: "Assignees", className: "assignees" },
  { label: "Tags", className: "tags" },
  { label: "Milestone", className: "milestone" },
  { label: "Status", className: "status" }
];

export const createModal = () => {
  const backdrop = modalBackdropTemplate();
  document.body.appendChild(backdrop);

  const close = () => {
    backdrop.classList.remove("open");
    backdrop.innerHTML = "";
  };

  const buildDetail = (issue) => {
    const modal = modalTemplate();
    modal.querySelector('[data-field="heading"]').textContent =
      `${issue.title} (${issue.id})`;
    modal.querySelector('[data-field="titleBox"]').textContent =
      `${issue.title} (${issue.id})`;
    modal.querySelector('[data-field="description"]').textContent =
      issue.description || "No description provided.";

    const sidebar = modal.querySelector('[data-role="sidebar"]');
    pillConfig.forEach((pill) => {
      sidebar.appendChild(pillTemplate(pill.label, pill.className));
    });

    const commentsWrap = modal.querySelector('[data-role="comments"]');
    (issue.comments || []).forEach((comment) => {
      commentsWrap.appendChild(commentCardTemplate(comment));
    });

    modal.querySelector(".modal-close").addEventListener("click", close);
    return modal;
  };

  const open = (issue) => {
    backdrop.innerHTML = "";
    backdrop.appendChild(buildDetail(issue));
    backdrop.classList.add("open");
  };

  backdrop.addEventListener("click", (evt) => {
    if (evt.target === backdrop) {
      close();
    }
  });

  return { open, close };
};
