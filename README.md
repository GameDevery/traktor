# Traktor

## How to build

1. Ensure system are up to date for building Traktor.
```
sudo ./scripts/misc/install-linux-deps.sh
```

2. Ensure all 3rdp dependencies are installed or updated in the 3rdp directory.
```
./bin/linux/releasestatic/Traktor.Run.App ./scripts/misc/update-3rdp.run
```

3. Generate project files.
```
./scripts/build-projects-make-linux.sh
```

4. Build (using vscode integration script manually)
```
./scripts/build-vscode.sh Linux
```

---

## How to run

### Editor
```
./scripts/run-editor.sh
```

### Avalanche Server
The Avalanche Server is providing build artifact caching for quicker builds by sharing artifacts from previous builds by local and/or remote users.
```
./scripts/run-avalanche-server.sh
```

### Standalone Remote Server
Note, since each editor include an embedded server so this is only required on remote hosts which only need to be a deploy target.
```
./scripts/run-remote-server.sh
```