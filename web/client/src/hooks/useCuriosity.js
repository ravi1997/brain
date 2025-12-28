import { useState, useEffect } from 'react';

// Item 216, 217, 206: Curiosity Hook
export const useCuriosity = (ws) => {
    const [topic, setTopic] = useState("None");
    const [interestLevel, setInterestLevel] = useState(0.0);

    useEffect(() => {
        if (!ws) return;
        // Mock socket listener
        const interval = setInterval(() => {
            // Simulate random updates
            setInterestLevel(Math.random());
        }, 3000);
        return () => clearInterval(interval);
    }, [ws]);

    return { topic, interestLevel };
};
