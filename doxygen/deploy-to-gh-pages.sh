#!/bin/bash
set -o errexit

# config
git config --global user.email "t.fischer@imperial.ac.uk"
git config --global user.name "Tobias-Fischer"

git checkout gh-pages
git rebase master
cd doxygen
bash ./doc-compile.sh
git add -u .
git commit --amend --no-edit
git push --force-with-lease --quiet "https://${GITHUB_TOKEN}@$github.com/${GITHUB_REPO}.git" master:gh-pages > /dev/null 2>&1
git checkout master

