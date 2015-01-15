
package=rsq
version=0.0.2

targets=(rsq)
type[rsq]=binary
sources[rsq]="$(echo src/**/*.c)"

dist=(
	data/**/*.txt
	build.zsh build/*.zsh project.zsh
)

CC=clang
LDFLAGS=""
CFLAGS="-Wall -Wextra -pedantic -g"

