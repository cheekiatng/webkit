#!/bin/sh -ex
# we want to cd into assets because we want the path generated to be relative to the assets directory.
(cd assets;
	# delete any .DS_Store files 
	find . -name '*.DS_Store' -type f -delete;
	# List files only (no directories). Cut the leading ./
	# suite/ch07/7.6/7.6.1/7.6.1-2-10.js
	echo "Generating list to test262_filelist.txt"
	find ./suite -type f -name "*.js" -exec ls -l1 {} \; | cut -c 3- > test262_filelist.txt;
)


