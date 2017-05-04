#!/bin/bash
set -o errexit

# config
git config --global user.email "t.fischer@imperial.ac.uk"
git config --global user.name "Tobias-Fischer"

git checkout -b gh-pages
cd doxygen
bash ./doc-compile.sh
git add -f doc
git commit --message "Travis build: $TRAVIS_BUILD_NUMBER provided doxygen documentation"
git push --force --quiet "https://${GITHUB_TOKEN}@$github.com/${GITHUB_REPO}.git" master:gh-pages > /dev/null 2>&1
git checkout master

