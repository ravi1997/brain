import os
import random
import re
from common import logger, git_commit

BRAIN_CPP_PATH = "brain.cpp"

class ArchitectAgent:
    def __init__(self):
        self.name = "Architect"
    
    def plan_and_upgrade(self):
        logger.info(f"[{self.name}] Scanning for upgrade opportunities...")
        
        # simulated_intelligence: We will 'optimize' the energy decay rate
        # This represents the agent tuning its own parameters.
        try:
            self._mutate_energy_decay()
            return True
        except Exception as e:
            logger.error(f"[{self.name}] Upgrade failed: {str(e)}")
            return False

    def _mutate_energy_decay(self):
        with open(BRAIN_CPP_PATH, 'r') as f:
            content = f.read()
        
        # Matches: double energy_decay = 0.002;
        # Allowing for variations
        pattern = r"(double\s+energy_decay\s*=\s*)(\d+\.\d+)(\s*;)"
        match = re.search(pattern, content)
        
        if match:
            prefix, current_val, suffix = match.groups()
            current_val = float(current_val)
            
            # Mutate slightly (random walk)
            mutation = random.uniform(-0.0005, 0.0005)
            new_val = max(0.0001, min(0.01, current_val + mutation))
            
            new_content = re.sub(pattern, f"{prefix}{new_val:.6f}{suffix}", content)
            
            with open(BRAIN_CPP_PATH, 'w') as f:
                f.write(new_content)
                
            logger.info(f"[{self.name}] Optimized energy_decay_rate: {current_val:.6f} -> {new_val:.6f}")
            
            # Commit
            git_commit(f"feat(brain): Optimized energy_decay_rate to {new_val:.6f}")
        else:
            logger.warning(f"[{self.name}] Could not find parameter to optimize.")

if __name__ == "__main__":
    agent = ArchitectAgent()
    agent.plan_and_upgrade()
