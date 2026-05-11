from pathlib import Path
import importlib
import sys
import traceback


def _project_dir() -> Path:
    # <Project>/Plugins/TopGunMCP/Content/Python/init_unreal.py
    return Path(__file__).resolve().parents[4]


def _start_bridge() -> None:
    mcp_dir = _project_dir() / "Content" / "Developers" / "rober" / "unreal_mcp"
    if not mcp_dir.exists():
        raise FileNotFoundError(f"Unreal MCP directory not found: {mcp_dir}")

    mcp_dir_text = str(mcp_dir)
    if mcp_dir_text not in sys.path:
        sys.path.append(mcp_dir_text)

    import unreal_bridge

    unreal_bridge.stop_ticker()
    unreal_bridge = importlib.reload(unreal_bridge)
    unreal_bridge.stop_ticker()
    unreal_bridge.start_ticker(1.0)


try:
    _start_bridge()
except Exception:
    traceback.print_exc()
