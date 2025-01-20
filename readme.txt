Part of SCB
Utility to generate documentation from code
To run, copy config.def.h into config.h (cp config.def.h config.h), then compile main.c.
To customize, edit config.h:
	outputSuffix gets added to filenames to create the default file name used without -o
	headerMd is the markdown inserted by $b
