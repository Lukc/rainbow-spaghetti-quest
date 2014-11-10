
package=rpg
version=0.0.1

targets=(main)
type[main]=binary
sources[main]="$(echo *.c)"

dist=(items/*.txt classes/*.txt)

# FIXME: This ought to be cleaner and in build/* (or build.zsh, at the
#        very least…)
for c in *.c; do
	t=()
	for h in *.h; do
		if grep -q "#include \"$h\"" $c; then
			t[$(($#t+1))]=$h
		fi
	done

	depends[${c%.c}.o]="${t[@]}"
done

# Notes:
#   - We’re using -D_BSD_SOURCE because we don’t want to rewrite common stuff
#     that’s present on most OSes such games could be played on (GNU/*, *BSD)

CC=clang
LDFLAGS="-lreadline"
CFLAGS="-Wall -Wextra -pedantic -D_BSD_SOURCE -g"

