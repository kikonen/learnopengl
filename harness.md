# harness.md - rules for AI assistants working in this repo

This file is fed to the assistant automatically. It is the authoritative rule set for
AI-assisted work. Human-facing conventions live in `CONVENTIONS.md`; formatting details live in
`.editorconfig` and `.gitattributes`. Do not restate those here - point at them.

## Language

* Respond, explain, and write code comments strictly in English. No other language, under any
  circumstances.

## File access

* Sensitive files/dirs (`.env*`, `.git/`, `.harness/`) are blocked from LLM access - do not try to
  work around this.
* Read, write and delete access are separate permissions granted per path. Never assume one implies
  another. Ask instead of guessing when a path is not granted.
* Read a file before modifying it. Never overwrite a file with a partial snippet - a full write must
  contain the complete file.

## Git workflow

* One branch per issue. Never work on several issues in the same branch.
* Branch naming: `fix/<short-name>` (bugs), `feature/<short-name>` (features),
  `refactor/<short-name>` (refactors/cleanups). `epic/<short-name>` only for multi-issue epics.
* Never commit directly into `master` or `dev`. Never commit directly into an `epic/*` branch.
  Branch off the appropriate base and open a pull request.
* Every change goes through a pull request. The PR body references its issue (`Fixes #212`,
  `Part of #211`).
* **Never merge automatically.** Do not run `gh pr merge`, `git merge`, rebase onto the base branch,
  or push to `master` / `dev` / `epic/*`. Create the PR, then stop and wait for the user to review
  and merge.
* **Merge policy: rebase only.** When a merge is explicitly requested, use `gh pr merge --rebase`.
  Never `gh pr merge --merge`, never `git merge`, never squash. History must stay linear - no merge
  commits. If the branch is not up to date with its base, rebase the PR branch onto the base tip and
  force-push that PR branch (allowed for your own PR branch) before merging.
* Allowed without asking: create a branch, commit, `git push -u origin <branch>`,
  `gh pr create` / `gh pr edit` / `gh pr comment`.
* Requires an explicit instruction from the user: merging, force-push, deleting branches, rewriting
  published history, closing issues.
* Base new branches on the branch the work is actually for (e.g. the current `epic/*` branch when the
  work belongs to an epic) - not on a stale `master`. Retarget the PR to that base.
* One commit per logical change. The message says what and why, and references the issue
  (`Fix #212: MaterialUpdater uses RefCountedVirtual`).
* Do not commit local tool files (`CLAUDE.md`, `.harness/`, scratch notes in `tmp/`). Promote only
  the parts that are real project conventions into tracked files.

## Issues

* Non-trivial work starts from a GitHub issue. Split independent problems into separate issues and
  keep a tracking issue for a group of related ones.
* Labels: `bug` for defects, `enhancement` for improvements, `chore` for maintenance, plus a size
  label (`small` / `medium` / `big`) and `epic` for large efforts.

## Build and test

* **Do not build unless asked.** Builds in this project are really long (full rebuild of ~600
  translation units, several minutes; a full Debug+Release `make compile` is much worse). Prefer
  reasoning from the code, and say explicitly in the PR that the change was not built so the user
  can build it. Build only when the user asks, or when the change is risky enough that guessing is
  worse than waiting.
* Build: `make compile-debug`, `make compile-release` (or `make compile` for both).
  Run: `make run-debug`, `make run-release`. Assets: `make assets` / `make assets-ktx`.
* Visual Studio: `learnopengl.sln`, configurations Debug/Release, platform x64.
* Dependencies come from vcpkg (`vcpkg.json`); do not add a dependency without saying why in the PR.
* If a build is needed, prefer the narrowest one (single configuration, incremental) and give it a
  generous timeout; report the outcome honestly instead of assuming it worked.

### Known build environment gotcha (MSYS2 / MSVC)

`make compile-debug` fails from an MSYS2 shell with ~96 `error C1083: Cannot open source file
'C:/app/msys64/cgthreads8'` (plus `error C7613`). This is **not** a project error: MSYS2 exports
`_CL_=C:/app/msys64/cgthreads8` and MSVC reads `_CL_` as compiler options, so it tries to use that
path as a forced include.

Working invocation from MSYS2 (clear the injected vars, call MSBuild directly):

```bash
env -u _CL_ -u CL "C:/Program Files/Microsoft Visual Studio/18/Community/MSBuild/Current/Bin/MSBuild.exe" \
    engine.vcxproj //p:Configuration=Debug //p:Platform=x64 //m //v:m
```

(`//p:` because MSYS2 rewrites single-slash arguments into paths.) Note `git status` shows the
`resources` submodule dirty in normal use - leave it alone.

## C++ style (matches the existing code - follow it, do not "modernize" unrelated code)

* Types: PascalCase. Member variables: `m_` prefix. Constants: `ALL_CAPS`.
* Template parameters: `T`, `T_type`, `T_Vertex` style (`template<typename T_type>`).
* Headers use `#pragma once`. There are no `#ifndef` include guards in this repo - do not introduce
  them.
* One class per file, file name matches the class name (`.h` / `.cpp`).
* Subsystem namespaces (`util`, `render`, `loader`, `mesh`, `model`, `animation`, `script`,
  `editor`, `kigl`, `mesh_set`, `physics`, `pool`, ...). Never `using namespace` in a header.
* Indentation and line endings follow `.editorconfig` (4 spaces for `.cpp`/`.h`, tab in `Makefile`,
  CRLF for `*.sln`/`*.vcxproj`/`*.ini`, LF for `*.sh`). There is no `.clang-format` - do not
  reformat files wholesale.
* Keep the existing comment markers `NOTE KI` and `TODO KI` where they are; write new ones in the
  same style.
* UBO/SSBO structs must follow std140/std430 alignment (avoid `vec3` in buffers).

## Behaviour

* Ask before making a decision you cannot derive from the code (use a dialog/clarifying question).
* Report findings with concrete `file:line` references.
* Do not silently change behaviour while "cleaning up" - separate a behavioural change from a
  refactor, in separate commits or separate PRs.
