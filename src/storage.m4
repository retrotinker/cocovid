ifdef(`storage',
ifelse(storage, `vhd', include(`storage_vhd.m4'),
	storage, `ide', include(`storage_ide.m4'),
`errprint(`Unknown storage type!')m4exit(1)'),
`errprint(`Storage undefined!')m4exit(1)')
