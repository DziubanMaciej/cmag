# How to bump a version

1. Update version in [/cmag_core/core/version.h](version.h).
2. Update changelog in [/deploy/guest_scripts/debian/changelog](debian/changelog).



# How to create a release

1. Ensure the new version is already added to debian changelog.
2. Create a tag in git with cmag version prefix with a "v", e.g. "v0.1.0". Push this tag to GitHub
3. Call deploy script with version argument, e.g. `deploy.py 0.1.0`.
4. Watch the output. After all VMs are done there will be a report saying which ones failed.
5. Look at Launchpad after 10 minutes and check, whether all Ubuntu builds passed.
6. Create release on GitHub. Paste in the generated description from `assets/description.md` and upload other assets as files.
