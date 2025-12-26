import React, { useState } from 'react';
import Layout from './components/Layout';
import Dashboard from './components/Dashboard';
import Cognition from './components/Cognition';
import Terminal from './components/Terminal';
import System from './components/System';
import Settings from './components/Settings';

function App() {
  const [activeView, setActiveView] = useState('dashboard');
  const [theme, setTheme] = useState('dark');

  const toggleTheme = () => {
    const newTheme = theme === 'dark' ? 'light' : 'dark';
    setTheme(newTheme);
    document.documentElement.setAttribute('data-theme', newTheme);
  };

  const renderView = () => {
    switch(activeView) {
      case 'dashboard': return <Dashboard />;
      case 'neural': return <Cognition />;
      case 'terminal': return <Terminal />;
      case 'system': return <System />;
      case 'settings': return <Settings theme={theme} toggleTheme={toggleTheme} />;
      default: return <Dashboard />;
    }
  };

  return (
    <Layout activeView={activeView} setActiveView={setActiveView}>
      {renderView()}
    </Layout>
  );
}

export default App;
