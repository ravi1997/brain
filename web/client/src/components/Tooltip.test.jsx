import { render, screen, fireEvent } from '@testing-library/react';
import { describe, it, expect } from 'vitest';
import Tooltip from './Tooltip';
import React from 'react';

describe('Tooltip', () => {
    it('shows text on hover', () => {
        const testText = 'Hello Tooltip';
        render(
            <Tooltip text={testText}>
                <button>Hover Me</button>
            </Tooltip>
        );
        
        const trigger = screen.getByText('Hover Me');
        fireEvent.mouseEnter(trigger);
        
        expect(screen.getByText(testText)).toBeInTheDocument();
        
        fireEvent.mouseLeave(trigger);
        expect(screen.queryByText(testText)).not.toBeInTheDocument();
    });
});
