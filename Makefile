all:
	standardese -c args.ini ../cool/include/cool
	sh check_files.sh
