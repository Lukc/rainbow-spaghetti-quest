
package=rsq
version=0.0.2

targets=(rsq)
type[rsq]=binary
sources[rsq]="$(echo src/*.c)"

dist=(
	data/**/*.txt
	build.zsh build/*.zsh project.zsh
)

# Notes:
#   - We’re using -D_BSD_SOURCE because we don’t want to rewrite common stuff
#     that’s present on most OSes such games could be played on (GNU/*, *BSD)

CC=clang
LDFLAGS=""
CFLAGS="-Wall -Wextra -pedantic -g"

