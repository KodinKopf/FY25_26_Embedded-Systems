# FY25_26_Embedded-Systems

## Git Etiquette Guide

### What is Git

Git is a version control system that helps you track changes in your code over time. It lets multiple people work on the same project without overwriting each other’s work. With Git, you can save snapshots of your project (called commits), experiment safely in separate branches, and merge your changes back together when you’re ready.

We need Git so that we can each work on the codebase independently, while being able to easily merge our code together.

---

### Git Structure

#### Branches

* **main:** Stable production branch that will be used for the final pod.
* **dev:** Integration branch. Pretty much our top-level running development branch.
* **feature:** New branch for each task/issue. Should be merged into dev when done.

#### Rules to Remember

* Nobody commits directly to main. Nobody should be committing to dev directly either. You will open a PR (pull request) to dev.
* When starting a new feature branch, make sure to start it from dev since that will be the most up-to-date version of our code.
* To add a new feature, start from the dev branch and create a separate branch named `feature_[feature-name]`. Work on your changes in that branch, then open a pull request (PR) to merge it back into dev. A code review will take place before the PR is approved and merged, ensuring that all updates maintain quality and consistency in the codebase.

---

### What Does a Good Commit Message Look Like

A good commit message clearly explains what changed and why. It helps others (and your future self) understand the purpose of a change without digging through the code. Keep it short, but detailed enough to explain what the change is. Please don’t commit without a comment.

---

### Daily Work Routine

#### Get Latest from Remote

```bash
git fetch origin
```

#### Get on Dev Branch

```bash
git checkout <feature-branch>
```

#### Rebase

```bash
git rebase -i origin/dev
```

#### Once You Are Done Working, Push Your Changes to Your Feature Branch

```bash
git add <files>
git commit -m "Your message here"
git push
```

Please ALWAYS check what branch you are on!
Also, learning how to use the VSCode source control is EXTREMELY helpful (you don’t have to use the terminal).

---

### How to Open a PR (Pull Request)

1. Go to GitHub.
2. Open a PR from your feature branch into dev.
3. PR description should include:

   * What you changed
   * How to test
   * Related issue / subsystem (e.g., “Avionics/CAN”, “Controls/IMU”)
   * Assigned code owner(s)

---

### Git Command Cheat Sheet

```bash
# Get latest from remote
git fetch origin

# Get on dev branch
git checkout dev

# Rebase main on top of remote main
git rebase origin/main
# (or: git pull --rebase origin main)
```

---
