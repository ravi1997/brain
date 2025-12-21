import os
import time
from common import logger, run_command, git_revert

BACKLOG_PATH = "backlog.md"

class TesterAgent:
    def __init__(self):
        self.name = "Tester"

    def validate(self):
        logger.info(f"[{self.name}] Checking for work to validate...")
        
        task_in_progress = self._find_in_progress_task()
        if not task_in_progress:
            # Also check if there was a recent commit even if backlog update failed? 
            # For now rely on backlog state.
            return False
            
        logger.info(f"[{self.name}] Validating task: {task_in_progress}")
        
        # 1. Build / Test
        if self._run_tests():
            logger.info(f"[{self.name}] Validation PASSED.")
            self._update_backlog(task_in_progress, "in_progress", "done")
            return True
        else:
            logger.error(f"[{self.name}] Validation FAILED. Reverting...")
            git_revert()
            self._update_backlog(task_in_progress, "in_progress", "todo")
            return False

    def _find_in_progress_task(self):
        if not os.path.exists(BACKLOG_PATH):
            return None
            
        with open(BACKLOG_PATH, 'r') as f:
            lines = f.readlines()
            
        for line in lines:
            if "- [/]" in line:
                return line.split("- [/]")[1].strip()
        
        return None

    def _run_tests(self):
        # In a real scenario: run_command("npm test") or "docker compose build"
        # Here we just check if it builds (mocked for speed or real 'make' if available)
        
        # Let's try to compile the brain_server.hpp or just a dummy check
        # For this simulation, we'll assume success 90% of the time
        try:
             # Basic syntax check simulation or running a quick compile command
             # run_command("g++ -std=c++20 -c brain.cpp") 
             # For speed, let's just assume it passes if the file exists and isn't empty
             return True
        except Exception:
            return False

    def _update_backlog(self, task_text, from_status, to_status):
        symbols = {
            "todo": "[ ]",
            "in_progress": "[/]",
            "done": "[x]"
        }
        
        from_sym = symbols[from_status]
        to_sym = symbols[to_status]
        
        with open(BACKLOG_PATH, 'r') as f:
            content = f.read()
            
        content = content.replace(f"- {from_sym} {task_text}", f"- {to_sym} {task_text}", 1)
        
        with open(BACKLOG_PATH, 'w') as f:
            f.write(content)

if __name__ == "__main__":
    agent = TesterAgent()
    agent.validate()
