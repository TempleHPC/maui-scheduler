#!/bin/bash
GH_REPO_REF=github.com/${TRAVIS_REPO_SLUG}.git

###########################################
# generate HTML documentation using Sphinx
###########################################
git clone -b gh-pages https://${GH_REPO_REF} gh-pages

# configure git user to be Travis CI
git config --global push.default simple
git config --global user.name "Travis CI"
git config --global user.email "travis@travis-ci.org"

# clear branch contents
cd gh-pages
rm -rf *
touch .nojekyll

# build HTML (Sphinx)
cd ../docs
make guide

# build HTML (Doxygen)
make refman

# commit to gh-pages branch
mv rst/_build/html ../gh-pages/docs
mv refman ../gh-pages/doxygen
cd ../gh-pages

if [ "$TRAVIS_BRANCH" != "master" ]
then
  echo "This commit was made against $TRAVIS_BRANCH and not the master! Not deploying updated documentation!"
  exit 0
fi

if [ -d "docs" ] && [ -f "docs/index.html" ]; then
    git add --all
    git commit -m "Deploy docs to GitHub Pages Travis build: ${TRAVIS_BUILD_NUMBER}" -m "Commit: ${TRAVIS_COMMIT}"
    git push --force "https://${GH_REPO_TOKEN}@${GH_REPO_REF}" > /dev/null 2>&1
else
    echo '' >&2
    echo 'Warning: No documentation (html) files have been found!' >&2
    echo 'Warning: Not going to push the documentation to GitHub!' >&2
    exit 1
fi
