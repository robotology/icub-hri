#!/bin/bash
set -o errexit

# config
git config --global user.email "t.fischer@imperial.ac.uk"
git config --global user.name "Tobias-Fischer"

git checkout -b gh-pages
cd doxygen
bash ./doc-compile.sh
bash ./write-index.sh
git add ../index.html
git add -f doc
git commit --quiet --message "provided doxygen documentation (Travis build $TRAVIS_BUILD_NUMBER)"
git remote add origin-pages https://${GITHUB_TOKEN}@github.com/${GITHUB_REPO}.git > /dev/null 2>&1
git push --force --quiet --set-upstream origin-pages gh-pages > /dev/null 2>&1

