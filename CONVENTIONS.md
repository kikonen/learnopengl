* Always respond, explain, and write comments strictly in English.
* Do not use any other language under any circumstances.

## Git workflow (issue-driven work)

* One branch per issue. Never work on several issues in the same branch.
* Branch naming follows the existing style:
  * `fix/<short-name>` for bugs (e.g. `fix/ref-count-underflow`)
  * `feature/<short-name>` for new features
  * `refactor/<short-name>` for refactors / cleanups
  * `epic/<short-name>` only for large multi-issue epics
* Never commit directly into `master` (or `dev`), and never commit directly into an `epic/*` branch.
  Branch off the appropriate base and open a pull request instead.
* Every change goes through a pull request. The PR body references the issue it addresses
  (`Fixes #212`, `Part of #211`).
* Never merge automatically. Do not run `gh pr merge`, `git merge`, `git rebase` onto the base
  branch, or push to `master` / `dev` / `epic/*`. After creating the PR, stop and wait for the user
  to review and merge.
* Allowed without asking: create the branch, commit, `git push -u origin <branch>`, `gh pr create`,
  `gh pr edit`, `gh pr comment`.
* Requires an explicit instruction from the user: merging, force-push, deleting branches, rewriting
  published history, closing issues.
* One commit per logical change, message states what and why (e.g. `Fix #212: MaterialUpdater uses
  RefCountedVirtual`).
