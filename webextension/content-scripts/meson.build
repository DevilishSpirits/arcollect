foreach file: webext_content_scripts
	webext_zip_cmds += 'content-scripts'/file
	webext_zip_depend_files += configure_file(output: file, input: file, copy: true)
endforeach
