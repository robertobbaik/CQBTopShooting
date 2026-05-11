# TopGun MCP Bridge Plugin

This project plugin starts the Unreal-side MCP bridge automatically when Unreal Editor initializes Python.

The plugin uses Unreal's standard Python plugin startup behavior:

- enabled plugin contributes `Content/Python` to Python startup paths
- Unreal runs `Content/Python/init_unreal.py`
- `init_unreal.py` loads `Content/Developers/rober/unreal_mcp/unreal_bridge.py`
- the bridge starts a non-blocking ticker and processes `Saved/UnrealMCP/commands`

The MCP stdio server remains at:

```text
Content/Developers/rober/unreal_mcp/server.py
```

Manual execution of `run_in_unreal.py` is now only a fallback for an already-open editor session.
