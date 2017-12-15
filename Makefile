all:
	standardese --output.format commonmark --input.blacklist_namespace detail --output.advanced_code_block=0 --input.extract_private=0 -I ../cool/include ../cool/include/cool
	sh check_files.sh
