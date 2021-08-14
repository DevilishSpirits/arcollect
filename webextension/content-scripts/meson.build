foreach file: webext_content_scripts
	webext_zip_cmds += 'content-scripts'/file
	webext_zip_depends += custom_target(file, command: python_copy_command, input: file, output: file)
endforeach
