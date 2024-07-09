# runc
Execute source files based on a static configuration

## Building
```sh
./configure
make
```

## Usage
`runc /path/to/code`

Example configurations are located in the `config` folder, runc reads two config directories, `/etc/runc.d` and `~/.config/runc`.

The configuration has three valid fields:
- `compiler`, the compiler command (required)
- `args`, compiler arguments (optional)
- `fileext`, the source file extension (required)
