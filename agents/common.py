import subprocess
import logging
import os
import sys

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - [%(levelname)s] - %(message)s',
    handlers=[
        logging.StreamHandler(sys.stdout),
        logging.FileHandler('../brain.log') # Log to main brain log for dashboard visibility
    ]
)
logger = logging.getLogger("AUTONOMY")

def run_command(command, cwd=None):
    """Runs a shell command and returns output."""
    try:
        result = subprocess.run(
            command, 
            shell=True, 
            check=True, 
            stdout=subprocess.PIPE, 
            stderr=subprocess.PIPE,
            cwd=cwd, # Use provided CWD or inherit current process CWD
            text=True
        )
        return result.stdout.strip()
    except subprocess.CalledProcessError as e:
        logger.error(f"Command failed: {command}\nError: {e.stderr}")
        raise e

def git_commit(message):
    """Commits changes to git."""
    try:
        run_command("git add .")
        run_command(f'git commit -m "{message}"')
        logger.info(f"Committed: {message}")
        return True
    except Exception as e:
        logger.error(f"Git commit failed: {e}")
        return False

def git_revert():
    """Reverts the last commit."""
    try:
        run_command("git revert --no-edit HEAD")
        logger.info("Reverted last commit.")
        return True
    except Exception as e:
        logger.error(f"Git revert failed: {e}")
        return False
