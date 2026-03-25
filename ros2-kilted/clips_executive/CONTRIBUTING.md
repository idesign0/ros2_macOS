# Contribution Guidelines

As an open-source project, we welcome and encourage the community to submit patches directly to the ROS2 CLIPS-Executive (CX).

Contributions should be made under the predominant license of the package you are contributing to. Entirely new packages should be made available under the Apache 2.0 license.

A license tells you what rights you have as a developer, as provided by the copyright holder. It is important that the contributor fully understands the licensing rights and agrees to them. Sometimes the copyright holder isn’t the contributor, such as when the contributor is doing work on behalf of a company.

## Signing Your Contributions with the Developer Certificate of Origin (DCO)

To contribute to this project, you must sign off your commits using the Developer Certificate of Origin (DCO). The DCO is a simple statement that you, as a contributor, have the legal right to submit your work under the project’s license.

When you sign off a commit, you add a Signed-off-by line to the commit message, which certifies the following

Developer Certificate of Origin 1.1

By making a contribution to this project, I certify that:

1) The contribution was created in whole or in part by me and I have the right to submit it under the open-source license indicated in the file; or

2) The contribution is based upon previous work that, to the best of my knowledge, is covered under an appropriate open-source license and I have the right to submit that work with modifications, whether created in whole or in part by me, under the same open-source license (unless I am permitted to submit under a different license), as indicated in the file; or

3) The contribution was provided directly to me by some other person who certified (1), (2) or (3) and I have not modified it.

4) I understand and agree that this project and the contribution are public and that a record of the contribution (including all personal information I submit with it, including my sign-off) is maintained indefinitely and may be redistributed consistent with this project or the open-source license(s) involved.



## How to Sign Off Your Commits

When creating a commit, add the -s or --signoff flag to your git commit command. This automatically adds the Signed-off-by line with your name and email address to the commit message.

For example:

```bash
git commit -s -m "Add awesome new feature"
```

This will produce a commit message like this:

```
Add awesome new feature

Signed-off-by: Your Name <your.email@example.com>
```

This project uses automated tools to verify that all commits are signed off. If a commit is missing a sign-off, the pull request will be flagged, and you will need to fix it before it can be merged.
