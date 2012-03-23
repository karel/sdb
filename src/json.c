/* Copyleft 2012 - sdb (aka SimpleDB) - pancake<nopcode.org> */

#include "sdb.h"
#include "json/json.h"

char *sdb_json_get (Sdb *s, const char *k, const char *p) {
	char *v = sdb_get (s, k);
	if (!v) return NULL;
	Rangstr rs = json_get (v, p);
	return rangstr_dup (&rs);
}

int sdb_json_geti (Sdb *s, const char *k, const char *p) {
	char *v = sdb_get (s, k);
	if (v) {
		Rangstr rs = json_get (v, p);
		return rangstr_int (&rs);
	}
	return 0;
}

int sdb_json_seti (Sdb *s, const char *k, const char *p, int v) {
	char str[64];
	sprintf (str, "%d", v);
	return sdb_json_set (s, k, p, str);
}

int sdb_json_set (Sdb *s, const char *k, const char *p, const char *v) {
	const char *beg[3];
	const char *end[3];
	int idx, len[3];
	char *str = NULL;
	Rangstr rs;

	char *js = sdb_get (s, k);
	if (!js) return 0;
	rs = json_get (js, p);
	if (!rs.p) {
		printf ("FAIL\n");
		free (js);
		return 0;
	}
#define WLEN(x) (int)(size_t)(end[x]-beg[x])

	beg[0] = js;
	end[0] = rs.p + rs.f;
	len[0] = WLEN (0);

	beg[1] = v;
	end[1] = v + strlen (v);
	len[1] = WLEN (1);

	beg[2] = rs.p + rs.t;
	end[2] = js + strlen (js);
	len[2] = WLEN (2);

	// TODO: accelerate with small buffer in stack for small jsons
	str = malloc (len[0]+len[1]+len[2]+1);
	idx = len[0];
	memcpy (str, beg[0], idx);
	memcpy (str+idx, beg[1], len[1]);
	idx += len[1];
	memcpy (str+idx, beg[2], len[2]);
	str[idx+len[2]] = 0;

	sdb_set (s, k, str);
	free (str);
	free (js);
	return 1;
}

char *sdb_json_indent(const char *s) {
	int indent = -1;
	int i, instr = 0;
	char *o, *O = malloc (strlen (s)*2);
	for (o=O; *s; s++) {
		if (instr) {
			if (s[0] == '"') {
				instr = 0;
			} else {
				if (s[0] == '\\' && s[1] == '"')
					*o++ = *s;
			}
			*o++ = *s;
			continue;
		} else {
			if (s[0] == '"')
				instr = 1;
		}
		if (*s == '\n'|| *s == '\r' || *s == '\t' || *s == ' ')
			continue;
		#define INDENT(x) indent+=x; for (i=0;i<indent;i++) *o++ = '\t'
		switch (*s) {
		case ',':
			*o++ = *s;
			*o++ = '\n';
			INDENT(0);
			break;
		case '{':
		case '[':
			*o++ = *s;
			*o++ = '\n';
			INDENT (1);
			break;
		case '}':
		case ']':
			*o++ = *s;
			*o++ = '\n';
			INDENT (-1);
			break;
		default:
			*o++ = *s;
		}
	}
	return O;
}

char *sdb_json_unindent(const char *s) {
	int instr = 0;
	char *o, *O = malloc (strlen (s));
	for (o=O; *s; s++) {
		if (instr) {
			if (s[0] == '"') {
				instr = 0;
			} else {
				if (s[0] == '\\' && s[1] == '"')
					*o++ = *s;
			}
			*o++ = *s;
			continue;
		} else {
			if (s[0] == '"')
				instr = 1;
		}
		if (*s == '\n'|| *s == '\r' || *s == '\t' || *s == ' ')
			continue;
		*o++ = *s;
	}
	return O;
}
