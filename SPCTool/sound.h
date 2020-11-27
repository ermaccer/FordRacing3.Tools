#pragma once


struct spc_header {
	int  entries;
};

struct spc_entry {
	char	name[32] = {};
	int		params[9];
	int	    baseOffset;
	int     size;
};