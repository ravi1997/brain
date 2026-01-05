# User Guide: Brain Simulation

## Introduction
The Brain Simulation is an interactive AI platform that mimics a biological brain. It experiences needs like hunger and thirst, has emotions, and can converse with you. This guide will help you install and interact with your digital brain.

## Key Features
- **Conversational AI**: Chat with the brain in natural language.
- **Biological Needs**: Watch as the brain gets hungry, thirsty, or tired.
- **Emotional State**: Observe how the brain's mood changes based on its experiences.
- **Visual Dashboard**: A rich web interface to monitor all brain activities in real-time.

## Installation / Getting Started

### 1. Requirements
- A modern web browser (Chrome, Firefox, Safari).
- Docker Desktop (recommended) OR basic terminal skills.

### 2. Quick Start (Standard)
1.  Open your terminal.
2.  Navigate to the project folder.
3.  Run the start script (if available) or use Docker:
    ```bash
    docker-compose up
    ```
4.  Open your browser to `http://localhost:3000`.

## How-To Guides

### 1. Talking to the Brain
1.  Launch the application in your browser.
2.  Locate the chat window on the right side.
3.  Type a message (e.g., "Hello, how are you?") and press Enter.
4.  The brain will process your input (showing "Thinking...") and respond.

### 2. Monitoring Needs
1.  Look at the "Biological State" panel.
2.  You will see bars for Hunger, Thirst, and Energy.
3.  If a bar gets too low (or high, depending on the metric), the brain might complain!

### 3. Understanding Emotions
1.  Check the "Emotional State" radar chart.
2.  It shows current levels of Happiness, Sadness, Anger, and Fear.
3.  Interactions and needs affect these levels.

## FAQs

**Q: Why isn't the brain responding?**
A: It might be "sleeping" or processing a heavy thought. Check if the backend server is running.

**Q: Can I reset the brain?**
A: Yes, restarting the application usually resets the short-term state.

## Troubleshooting
- **White Screen**: Refresh the page.
- **Connection Error**: Ensure the backend server is running (`localhost:8080`).

## Support
For issues, please open a ticket in the project repository or contact the development team.

---
**Version:** 1.0.0
**Last Updated:** 2026-01-05
