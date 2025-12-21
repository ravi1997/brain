import time
import sys
from architect import ArchitectAgent
from qa import QAAgent
from common import logger

def main_loop():
    architect = ArchitectAgent()
    qa = QAAgent()
    
    logger.info("=== CORTEX AUTONOMY PROTOCOL INITIATED ===")
    
    cycle = 0
    try:
        while True:
            cycle += 1
            logger.info(f"\n--- Evolution Cycle #{cycle} ---")
            
            # 1. Plan & Upgrade
            architect.plan_and_upgrade()
            
            # 2. Validate
            success = qa.validate()
            
            if not success:
                logger.warning("Upgrade failed validation. Initiating repairs...")
                qa.fix()
            else:
                logger.info("Upgrade verified and deployed.")
            
            # 3. Rest
            logger.info("Standby for 60 seconds...")
            time.sleep(60)
            
    except KeyboardInterrupt:
        logger.info("\n=== PROTOCOL TERMINATED ===")
        sys.exit(0)

if __name__ == "__main__":
    main_loop()
