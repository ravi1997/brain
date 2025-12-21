import time
import sys
from developer import DeveloperAgent
from tester import TesterAgent
from common import logger

def run_loop():
    developer = DeveloperAgent()
    tester = TesterAgent()
    
    logger.info("=== MULTI-AGENT DEV TEAM ONLINE ===")
    logger.info("Monitoring backlog.md for tasks...")
    
    cycle = 0
    try:
        while True:
            cycle += 1
            logger.info(f"\n--- Cycle #{cycle} ---")
            
            # 1. Developer attempts to work
            did_work = developer.work()
            
            if did_work:
                logger.info("Developer submitted work. Starting validation...")
                # 2. Tester validates
                tester.validate()
            else:
                logger.info("No work done (Backlog empty or low priority?). Checking for pending validation...")
                # Check if there's something pending validation anyway
                tester.validate()
            
            # 3. Rest
            logger.info("Standby for 10 seconds...")
            time.sleep(10)
            
    except KeyboardInterrupt:
        logger.info("\n=== DEV TEAM OFFLINE ===")
        sys.exit(0)

if __name__ == "__main__":
    run_loop()
