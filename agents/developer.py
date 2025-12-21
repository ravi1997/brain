import os
import re
import random
import time
from common import logger, git_commit

BACKLOG_PATH = "backlog.md"
BRAIN_CPP_PATH = "brain.cpp"

class DeveloperAgent:
    def __init__(self):
        self.name = "Developer"

    def work(self):
        logger.info(f"[{self.name}] Checking backlog for tasks...")
        
        task = self._pick_task()
        if not task:
            logger.info(f"[{self.name}] No high priority tasks found.")
            return False
            
        logger.info(f"[{self.name}] Picked task: {task}")
        
        # 1. Mark in progress
        self._update_backlog(task, "todo", "in_progress")
        
        # 2. Simulate work
        try:
            self._write_code(task)
            # 3. Commit
            git_commit(f"feat: {task} (implemented by {self.name})")
            logger.info(f"[{self.name}] Committed implementation for '{task}'")
            return True
        except Exception as e:
            logger.error(f"[{self.name}] Failed to implement '{task}': {e}")
            # Revert backlog status if failed
            self._update_backlog(task, "in_progress", "todo")
            return False

    def _pick_task(self):
        if not os.path.exists(BACKLOG_PATH):
            return None
            
        with open(BACKLOG_PATH, 'r') as f:
            lines = f.readlines()
            
        # Parse entire file for first available task
        for line in lines:
            if "- [ ]" in line:
                # Extract task text
                return line.split("- [ ]")[1].strip()
        
        return None

    def _update_backlog(self, task_text, from_status, to_status):
        # map status to symbols
        symbols = {
            "todo": "[ ]",
            "in_progress": "[/]",
            "done": "[x]"
        }
        
        from_sym = symbols[from_status]
        to_sym = symbols[to_status]
        
        with open(BACKLOG_PATH, 'r') as f:
            content = f.read()
            
        # Replace the specific line
        # Note: This is a simple replacement, might be risky if duplicate tasks exist
        content = content.replace(f"- {from_sym} {task_text}", f"- {to_sym} {task_text}", 1)
        
        with open(BACKLOG_PATH, 'w') as f:
            f.write(content)

    def _write_code(self, task):
        # Heuristic simulation of coding
        logger.info(f"[{self.name}] Writing code for: {task}...")
        
        # For demonstration, we'll append a comment to brain.cpp or a dummy file
        # capturing the "work" done.
        
        timestamp = time.strftime("%Y-%m-%d %H:%M:%S")
        comment = f"\n// [Dev] Implemented '{task}' at {timestamp}\n"
        
        with open(BRAIN_CPP_PATH, 'a') as f:
            f.write(comment)
            
        time.sleep(2) # simulate thinking

if __name__ == "__main__":
    agent = DeveloperAgent()
    agent.work()
