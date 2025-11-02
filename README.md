# FY25_26_Embedded-Systems
## Git Etiquette Guide:
What is Git?

Git is a version control system that helps you track changes in your code over time. It lets multiple people work on the same project without overwriting each other’s work. With Git, you can save snapshots of your project (called commits), experiment safely in separate branches, and merge your changes back together when you’re ready.

We need Git so that we can each work on the codebase independently, while being able to easily merge our code together.

Git Structure

Branches:

main: Stable production branch that will be used for the final pod.
dev: Integration branch. Pretty much our top level running development branch.
feature: New branch for each task/issue. Should be merged into dev when done.

Rules to remember:

Nobody commits directly to main. Nobody should be committing to dev directly either. You will open a PR (pull request) to dev. When starting a new feature branch, make sure to start it from dev since that will be the most up to date version of our code.
To add a new feature, start from the dev branch and create a separate branch named feature_[feature-name]. Work on your changes in that branch, then open a pull request (PR) to merge it back into dev. A code review will take place before the PR is approved and merged, ensuring that all updates maintain quality and consistency in the codebase.

What does a good commit message look like?
A good commit message clearly explains what changed and why. It helps others (and your future self) understand the purpose of a change without digging through the code. Keep it short, but detailed enough to explain what the change is. Please don’t commit without a comment.

Daily work routine

Get latest from remote
git fetch origin

Get on dev branch
git checkout <feature-branch>

Rebase 
git rebase -i origin/dev

Once you are done working, PUSH your changes to your feature branch using 

git add <files>
git commit -m “Your message here”
git push

Please ALWAYS check what branch you are on! Also, learning how to use the VSCode source control is EXTREMELY helpful (you don’t have to use the terminal). 
How to open a PR (pull request)
Go to GitHub.


Open PR from your feature branch into dev


PR description should include:


What you changed


How to test


Related issue / subsystem (e.g. “Avionics/CAN”, “Controls/IMU”)


Assign code owner(s)


Git command cheat sheet
