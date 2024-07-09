# runc
Execute source files based on a static configuration

## Building
First make sure you have [extlib](https://codeberg.org/axtlos/extlib) installed.

```sh
./configure
make
```

## Usage
`runc /path/to/code` or just add runc as the shebang to the source file

Example configurations are located in the `config` folder, runc reads two config directories, `/etc/runc.d` and `~/.config/runc`.

The configuration has three valid fields:
- `compiler`, the compiler command (required)
- `args`, compiler arguments (optional)
- `fileext`, the source file extension (required)
