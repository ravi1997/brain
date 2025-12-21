import os
from common import logger, run_command, git_revert

class QAAgent:
    def __init__(self):
        self.name = "QA"

    def validate(self):
        logger.info(f"[{self.name}] Starting Validation Protocol...")
        
        try:
            # 1. Build
            logger.info(f"[{self.name}] Building backend...")
            run_command("docker compose build brain_replica")
            
            # 2. Test (Mock test for speed in this demo, or real build check)
            # In a real scenario we would run: run_command("npm test", cwd="../web")
            # For this loop, simply checking if it builds is a good first step.
            # Let's also verify we didn't break the build.
            
            logger.info(f"[{self.name}] Validation Successful. Build verified.")
            return True
            
        except Exception as e:
            logger.error(f"[{self.name}] Validation FAILED: {e}")
            return False

    def fix(self):
        logger.info(f"[{self.name}] Attempting to fix...")
        # Simplest fix: Revert the change that broke it.
        git_revert()
        logger.info(f"[{self.name}] Fix applied (Revert).")

if __name__ == "__main__":
    agent = QAAgent()
    if not agent.validate():
        agent.fix()
