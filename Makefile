all:
	/data/Projects/standardese/build/tool/standardese -c args.ini ../cool/include/cool
	sh check_files.sh
