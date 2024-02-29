# How to create a release

1. Create a tag in git with cmag version prefix with a "v", e.g. "v0.1.0". Push this tag to GitHub
2. Call deploy script with version argument, e.g. `deploy.py 0.1.0`.
3. Watch the output. After all VMs are done there will be a report saying which ones failed.
4. Look at Launchpad after 10 minutes and check, whether all Ubuntu builds passed.
5. Create release on GitHub. Paste in the generated description from `assets/description.md` and upload other assets as files.
