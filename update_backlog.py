
import re

backlog_path = "/home/ravi/workspace/brain/backlog.md"

with open(backlog_path, "r") as f:
    content = f.read()

# Categories and their patterns to mark as [x]
patterns = [
    r"Refactor dashboard to use React components",
    r"Implement redis caching for frequent queries",
    r"Optimize C\+\+ compile times with precompiled headers",
    r"Implement fallback responses for unknown inputs",
    r"Add robust error handling for websocket disconnections",
    r"Add sentiment analysis to input processing",
    r"Add decay factor to emotional states",
    r"Add dark mode toggle to UI",
    r"Add 'Focus' mechanism to filter sensory input",
    r"Add support for multi-turn conversation context",
    r"Visualize real-time neuron activity with WebGL",
    r"Implement websocket reconnection logic",
    r"Create settings page for brain configuration",
    r"Add health check endpoint for monitoring",
    r"Dockerize build environment",
    r"Refactor memory retrieval for O\(1\) access",
    r"Modernize UI"
]

def mark_done(match):
    return "- [x]" + match.group(1)

for pattern in patterns:
    # Match "- [ ] [ANYTHING] PATTERN"
    # Added \s* to handle potential spaces
    full_pattern = re.compile(r"- \[ \](.*" + pattern + r".*)", re.IGNORECASE)
    content = full_pattern.sub(mark_done, content)

with open(backlog_path, "w") as f:
    f.write(content)

print("Expanded bulk update complete.")
