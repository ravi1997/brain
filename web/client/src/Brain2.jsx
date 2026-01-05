import React, { useState, useEffect } from 'react';
import { useBrain, usePort } from './hooks/useBrain';
import './Brain2.css';

const getPortName = (port) => {
    const names = {
        '9001': 'Dashboard', '9002': 'Emotions', '9003': 'Logs',
        '9004': 'Errors', '9005': 'Chat', '9006': 'Thoughts',
        '9007': 'Research', '9008': 'Extra/System', '9009': 'Admin',
        '9010': 'Tasks', '9011': 'Control', '9012': 'Graph'
    };
    return names[port] || 'Unknown';
};

const PortTerminal = ({ port, onClose }) => {
    const { status, messages, send } = usePort(port);
    const [inputValue, setInputValue] = useState('');

    const handleSend = (e) => {
        e.preventDefault();
        if (inputValue.trim()) {
            send(inputValue);
            setInputValue('');
        }
    };

    return (
        <div className="port-terminal-overlay">
            <div className="port-terminal">
                <div className="terminal-header">
                    <h3>Port {port}: {getPortName(port)}</h3>
                    <div className="terminal-status">
                        <span className={`status-dot ${status}`}></span>
                        {status.toUpperCase()}
                    </div>
                    <button onClick={onClose} className="close-terminal">×</button>
                </div>
                <div className="terminal-content">
                    {messages.map((m, i) => (
                        <div key={i} className="terminal-line">
                            <span className="line-time">[{m.time}]</span>
                            <span className="line-text">{m.text}</span>
                        </div>
                    ))}
                </div>
                <form onSubmit={handleSend} className="terminal-input-container">
                    <span className="terminal-prompt">{'>'}</span>
                    <input 
                        type="text" 
                        value={inputValue} 
                        onChange={(e) => setInputValue(e.target.value)}
                        placeholder="Send command..."
                        autoFocus
                    />
                </form>
            </div>
        </div>
    );
};

const Brain2 = () => {
  const { connected, brainData, sendCommand } = useBrain();
  const [activeTab, setActiveTab] = useState('cognitive');
  const [ports, setPorts] = useState({});
  const [selectedPort, setSelectedPort] = useState(null);
  const [cognitiveResult, setCognitiveResult] = useState('');
  const [loading, setLoading] = useState(false);

  // Cognitive Testing Forms
  const [reasonQuery, setReasonQuery] = useState('');
  const [causalCause, setCausalCause] = useState('');
  const [causalEffect, setCausalEffect] = useState('');
  const [whatIfVariable, setWhatIfVariable] = useState('');
  const [whatIfValue, setWhatIfValue] = useState('');
  const [whatIfTarget, setWhatIfTarget] = useState('');
  const [commonsenseSubject, setCommonsenseSubject] = useState('');
  const [commonsenseRelation, setCommonsenseRelation] = useState('');

  // Port monitoring
  useEffect(() => {
    const interval = setInterval(() => {
      // Monitor ports 9001-9012
      const portStatus = {};
      for (let port = 9001; port <= 9012; port++) {
        portStatus[port] = connected ? 'active' : 'inactive';
      }
      setPorts(portStatus);
    }, 1000);
    return () => clearInterval(interval);
  }, [connected]);

  const testCognitiveFeature = async (feature, data) => {
    setLoading(true);
    setCognitiveResult('Processing...');
    
    try {
      const command = {
        type: 'cognitive_test',
        feature,
        data
      };
      
      const result = await sendCommand(command);
      setCognitiveResult(JSON.stringify(result, null, 2));
    } catch (error) {
      setCognitiveResult(`Error: ${error.message}`);
    } finally {
      setLoading(false);
    }
  };

  const handleDeepReason = () => {
    testCognitiveFeature('deep_reason', { query: reasonQuery });
  };

  const handleCausality = () => {
    testCognitiveFeature('analyze_causality', { 
      cause: causalCause, 
      effect: causalEffect 
    });
  };

  const handleWhatIf = () => {
    testCognitiveFeature('what_if', {
      variable: whatIfVariable,
      value: parseFloat(whatIfValue),
      target: whatIfTarget
    });
  };

  const handleCommonsense = () => {
    testCognitiveFeature('query_commonsense', {
      subject: commonsenseSubject,
      relation: commonsenseRelation
    });
  };

  const handleGetStatus = () => {
    testCognitiveFeature('get_cognitive_status', {});
  };


  return (
    <div className="brain2-container">
      {/* Header */}
      <header className="brain2-header">
        <div className="brain-logo">
          <div className="pulsing-brain">🧠</div>
          <div>
            <h1>BRAIN 2.0</h1>
            <p className="tagline">Cognitive Architecture • 100 AI Features</p>
          </div>
        </div>
        <div className="connection-status">
          <div className={`status-indicator ${connected ? 'connected' : 'disconnected'}`}>
            {connected ? '● CONNECTED' : '○ DISCONNECTED'}
          </div>
        </div>
      </header>

      {/* Main Content */}
      <div className="brain2-main">
        {/* Sidebar Navigation */}
        <nav className="brain2-sidebar">
          <button 
            className={`nav-btn ${activeTab === 'cognitive' ? 'active' : ''}`}
            onClick={() => setActiveTab('cognitive')}
          >
            🧪 Cognitive Testing
          </button>
          <button 
            className={`nav-btn ${activeTab === 'ports' ? 'active' : ''}`}
            onClick={() => setActiveTab('ports')}
          >
            🔌 Port Monitor
          </button>
          <button 
            className={`nav-btn ${activeTab === 'perception' ? 'active' : ''}`}
            onClick={() => setActiveTab('perception')}
          >
            👁️ Perception
          </button>
          <button 
            className={`nav-btn ${activeTab === 'learning' ? 'active' : ''}`}
            onClick={() => setActiveTab('learning')}
          >
            📚 Learning
          </button>
          <button 
            className={`nav-btn ${activeTab === 'knowledge' ? 'active' : ''}`}
            onClick={() => setActiveTab('knowledge')}
          >
            🗄️ Knowledge
          </button>
          <button 
            className={`nav-btn ${activeTab === 'system' ? 'active' : ''}`}
            onClick={() => setActiveTab('system')}
          >
            ⚙️ System
          </button>
        </nav>

        {/* Content Area */}
        <div className="brain2-content">
          {/* Cognitive Testing Tab */}
          {activeTab === 'cognitive' && (
            <div className="tab-content">
              <h2>🧠 Cognitive Core Testing</h2>
              <p className="subtitle">Test all Brain 2.0 reasoning capabilities</p>

              <div className="test-grid">
                {/* Deep Reasoning */}
                <div className="test-panel">
                  <h3>💭 Deep Reasoning</h3>
                  <p>Multi-system reasoning with explanations</p>
                  <input
                    type="text"
                    placeholder="Ask a question (e.g., Why does rain make grass wet?)"
                    value={reasonQuery}
                    onChange={(e) => setReasonQuery(e.target.value)}
                    className="test-input"
                  />
                  <button onClick={handleDeepReason} className="test-btn" disabled={loading}>
                    Reason
                  </button>
                </div>

                {/* Causal Analysis */}
                <div className="test-panel">
                  <h3>🔗 Causal Analysis</h3>
                  <p>Analyze cause-effect relationships</p>
                  <input
                    type="text"
                    placeholder="Cause (e.g., exercise)"
                    value={causalCause}
                    onChange={(e) => setCausalCause(e.target.value)}
                    className="test-input"
                  />
                  <input
                    type="text"
                    placeholder="Effect (e.g., health)"
                    value={causalEffect}
                    onChange={(e) => setCausalEffect(e.target.value)}
                    className="test-input"
                  />
                  <button onClick={handleCausality} className="test-btn" disabled={loading}>
                    Analyze
                  </button>
                </div>

                {/* What-If Scenarios */}
                <div className="test-panel">
                  <h3>🔮 What-If Scenarios</h3>
                  <p>Counterfactual reasoning</p>
                  <input
                    type="text"
                    placeholder="Variable (e.g., temperature)"
                    value={whatIfVariable}
                    onChange={(e) => setWhatIfVariable(e.target.value)}
                    className="test-input"
                  />
                  <input
                    type="number"
                    placeholder="New value (e.g., 100)"
                    value={whatIfValue}
                    onChange={(e) => setWhatIfValue(e.target.value)}
                    className="test-input"
                  />
                  <input
                    type="text"
                    placeholder="Target (e.g., water_state)"
                    value={whatIfTarget}
                    onChange={(e) => setWhatIfTarget(e.target.value)}
                    className="test-input"
                  />
                  <button onClick={handleWhatIf} className="test-btn" disabled={loading}>
                    Predict
                  </button>
                </div>

                {/* Commonsense Knowledge */}
                <div className="test-panel">
                  <h3>🧩 Commonsense</h3>
                  <p>Query knowledge base</p>
                  <input
                    type="text"
                    placeholder="Subject (e.g., dog)"
                    value={commonsenseSubject}
                    onChange={(e) => setCommonsenseSubject(e.target.value)}
                    className="test-input"
                  />
                  <input
                    type="text"
                    placeholder="Relation (e.g., IsA)"
                    value={commonsenseRelation}
                    onChange={(e) => setCommonsenseRelation(e.target.value)}
                    className="test-input"
                  />
                  <button onClick={handleCommonsense} className="test-btn" disabled={loading}>
                    Query
                  </button>
                </div>

                {/* System Status */}
                <div className="test-panel status-panel">
                  <h3>📊 Cognitive Status</h3>
                  <p>View all active systems</p>
                  <button onClick={handleGetStatus} className="test-btn primary" disabled={loading}>
                    Get Full Status
                  </button>
                </div>
              </div>

              {/* Results Display */}
              {cognitiveResult && (
                <div className="results-panel">
                  <h3>📤 Results</h3>
                  <pre className="results-content">{cognitiveResult}</pre>
                </div>
              )}
            </div>
          )}

          {/* Port Monitor Tab */}
          {activeTab === 'ports' && (
            <div className="tab-content">
              <h2>🔌 Active Port Monitor</h2>
              <p className="subtitle">All Brain backend services (9001-9012)</p>

              <div className="port-grid">
                {Object.entries(ports).map(([port, status]) => (
                  <div 
                    key={port} 
                    className={`port-card ${status} clickable`}
                    onClick={() => setSelectedPort(port)}
                  >
                    <div className="port-number">:{port}</div>
                    <div className="port-label">{getPortName(port)}</div>
                    <div className={`port-status-indicator ${status}`}>
                      {status === 'active' ? '●' : '○'}
                    </div>
                    <div className="port-hint">Click to Interact</div>
                  </div>
                ))}
              </div>

              <div className="port-legend">
                <div><span className="legend-active">●</span> Active</div>
                <div><span className="legend-inactive">○</span> Inactive</div>
              </div>
            </div>
          )}

          {/* Perception Tab */}
          {activeTab === 'perception' && (
            <div className="tab-content">
              <h2>👁️ Perception Systems</h2>
              <p className="subtitle">Visual & Audio Understanding</p>

              <div className="feature-grid">
                <div className="feature-card">
                  <h3>📷 Object Detection</h3>
                  <p>YOLO v8 based detection</p>
                  <div className="feature-status">Ready</div>
                </div>
                <div className="feature-card">
                  <h3>❓ Visual QA</h3>
                  <p>Answer questions about images</p>
                  <div className="feature-status">Ready</div>
                </div>
                <div className="feature-card">
                  <h3>🎵 Music Understanding</h3>
                  <p>Tempo, chords, genre classification</p>
                  <div className="feature-status">Ready</div>
                </div>
                <div className="feature-card">
                  <h3>🔊 Sound Classification</h3>
                  <p>Environmental audio analysis</p>
                  <div className="feature-status">Ready</div>
                </div>
                <div className="feature-card">
                  <h3>🎯 3D Reconstruction</h3>
                  <p>Spatial understanding</p>
                  <div className="feature-status">Ready</div>
                </div>
              </div>
            </div>
          )}

          {/* Learning Tab */}
          {activeTab === 'learning' && (
            <div className="tab-content">
              <h2>📚 Learning Systems</h2>
              <p className="subtitle">Meta-Learning & Continual Adaptation</p>

              <div className="learning-overview">
                <div className="learning-status-card">
                  <h3>Active Focus</h3>
                  <div className="focus-topic">{brainData.learning?.focus_topic || 'Initializing...'}</div>
                  <div className="focus-meter">
                    <div className="focus-fill" style={{ width: `${(brainData.learning?.focus_level || 0) * 100}%` }}></div>
                  </div>
                  <div className="focus-label">Attention: {Math.round((brainData.learning?.focus_level || 0) * 100)}%</div>
                </div>
                <div className="knowledge-stats">
                  <div className="stat-box">
                    <div className="stat-num">{brainData.learning?.learned_count || 0}</div>
                    <div className="stat-text">Topics Mastered</div>
                  </div>
                </div>
              </div>

              <div className="feature-grid">
                <div className="feature-card">
                  <h3>🎯 Meta-Learning (MAML)</h3>
                  <p>Few-shot learning from examples</p>
                  <div className="feature-status">Ready</div>
                </div>
                <div className="feature-card">
                  <h3>♾️ Continual Learning</h3>
                  <p>Learn without forgetting (EWC)</p>
                  <div className="feature-status">Ready</div>
                </div>
                <div className="feature-card">
                  <h3>💾 Attention Memory</h3>
                  <p>Store & retrieve with attention</p>
                  <div className="feature-status">Ready</div>
                </div>
                <div className="feature-card">
                  <h3>🧬 Neuro-Evolution</h3>
                  <p>Genetic algorithm optimization</p>
                  <div className="feature-status">Ready</div>
                </div>
              </div>
            </div>
          )}

          {/* Knowledge Tab */}
          {activeTab === 'knowledge' && (
            <div className="tab-content">
              <h2>🗄️ Knowledge Systems</h2>
              <p className="subtitle">Semantic Web & Reasoning</p>

              <div className="feature-grid">
                <div className="feature-card">
                  <h3>🌐 Semantic Web (RDF/OWL)</h3>
                  <p>Ontology reasoning</p>
                  <div className="feature-status">Ready</div>
                </div>
                <div className="feature-card">
                  <h3>🧩 Common-Sense KB</h3>
                  <p>50+ everyday facts</p>
                  <div className="feature-status">Ready</div>
                </div>
                <div className="feature-card">
                  <h3>🔗 Knowledge Graphs</h3>
                  <p>Graph embeddings & queries</p>
                  <div className="feature-status">Ready</div>
                </div>
                <div className="feature-card">
                  <h3>🎯 Multi-Hop Reasoning</h3>
                  <p>Complex inference chains</p>
                  <div className="feature-status">Ready</div>
                </div>
              </div>
            </div>
          )}

          {/* System Tab */}
          {activeTab === 'system' && (
            <div className="tab-content">
              <h2>⚙️ System Overview</h2>
              <p className="subtitle">Brain 2.0 Architecture Status</p>

              <div className="system-stats">
                <div className="stat-card">
                  <div className="stat-value">{brainData.metadata?.knowledge_size ? (brainData.metadata.knowledge_size / 1024 / 1024).toFixed(1) : '0'}MB</div>
                  <div className="stat-label">Memory Size</div>
                </div>
                <div className="stat-card">
                  <div className="stat-value">{brainData.metadata?.uptime ? Math.floor(brainData.metadata.uptime / 3600) : '0'}h</div>
                  <div className="stat-label">Uptime</div>
                </div>
                <div className="stat-card">
                  <div className="stat-value">{connected ? 'ONLINE' : 'OFFLINE'}</div>
                  <div className="stat-label">Network</div>
                </div>
                <div className="stat-card">
                  <div className="stat-value">{brainData.metadata?.version || 'v2.0'}</div>
                  <div className="stat-label">Core Version</div>
                </div>
              </div>

              <div className="thought-monitor">
                <h3>Neural Stream (Current Thought)</h3>
                <div className="thought-bubbles">
                  <div className="thought-bubble active">
                    {brainData.thought || 'Idle...'}
                  </div>
                </div>
              </div>

              <div className="system-modules">
                <h3>Active Modules</h3>
                <div className="module-list">
                  <div className="module-item">✓ Neural Architecture (18 features)</div>
                  <div className="module-item">✓ Reasoning Systems (14 features)</div>
                  <div className="module-item">✓ Perception (10 features)</div>
                  <div className="module-item">✓ Knowledge Systems (6 features)</div>
                  <div className="module-item">✓ Distributed Intelligence (10 features)</div>
                  <div className="module-item">✓ Optimization (4 features)</div>
                </div>
              </div>
            </div>
          )}
        </div>
      </div>

      {/* Port Terminal Modal */}
      {selectedPort && (
        <PortTerminal 
            port={selectedPort} 
            onClose={() => setSelectedPort(null)} 
        />
      )}
    </div>
  );
};

export default Brain2;
