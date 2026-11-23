import React, { useState, useEffect, useRef } from 'react';

export default function AutocompleteView({ onMeasureLatency }) {
  const [query, setQuery] = useState('');
  const [suggestions, setSuggestions] = useState([]);
  const [selectedIndex, setSelectedIndex] = useState(-1);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState(false);
  const debounceRef = useRef(null);

  const SAMPLES = ['pro', 'comp', 'algo', 'poly', 'str'];

  useEffect(() => {
    if (!query.trim()) {
      setSuggestions([]);
      setSelectedIndex(-1);
      setError(false);
      return;
    }

    clearTimeout(debounceRef.current);
    debounceRef.current = setTimeout(async () => {
      const start = performance.now();
      try {
        setLoading(true);
        const res = await fetch(`/suggest?mode=autocomplete&q=${encodeURIComponent(query.trim())}`);
        if (!res.ok) throw new Error('API failed');
        const data = await res.json();
        onMeasureLatency(`${(performance.now() - start).toFixed(1)} ms`);
        setSuggestions(data.suggestions || []);
        setError(false);
      } catch (err) {
        console.error(err);
        onMeasureLatency('offline');
        setError(true);
      } finally {
        setLoading(false);
      }
    }, 120);

    return () => clearTimeout(debounceRef.current);
  }, [query]);

  const handleKeyDown = (e) => {
    if (!suggestions.length) return;
    if (e.key === 'ArrowDown') {
      e.preventDefault();
      setSelectedIndex(prev => Math.min(prev + 1, suggestions.length - 1));
    } else if (e.key === 'ArrowUp') {
      e.preventDefault();
      setSelectedIndex(prev => Math.max(prev - 1, 0));
    } else if (e.key === 'Enter') {
      if (selectedIndex >= 0 && selectedIndex < suggestions.length) {
        e.preventDefault();
        setQuery(suggestions[selectedIndex]);
      }
    }
  };

  const renderHighlighted = (word, prefix) => {
    if (word.toLowerCase().startsWith(prefix.toLowerCase())) {
      const match = word.substring(0, prefix.length);
      const rest = word.substring(prefix.length);
      return (
        <>
          <span className="prefix-match">{match}</span>
          {rest}
        </>
      );
    }
    return word;
  };

  return (
    <>
      <div className="input-outer">
        <div className="input-wrapper">
          <div className="input-icon-box">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
              <circle cx="11" cy="11" r="8"></circle>
              <line x1="21" y1="21" x2="16.65" y2="16.65"></line>
            </svg>
          </div>
          <input
            type="text"
            id="queryInput"
            value={query}
            onChange={(e) => setQuery(e.target.value)}
            onKeyDown={handleKeyDown}
            placeholder="Type a prefix (e.g. 'pro')..."
            autoComplete="off"
            autoCorrect="off"
            autoCapitalize="off"
            spellCheck="false"
            autoFocus
          />
          <div className="input-actions">
            <span className="kbd-hint">{query.length > 0 ? `${query.length}ch` : 'live'}</span>
            {query.length > 0 && (
              <button className="clear-btn" onClick={() => setQuery('')} aria-label="Clear">
                &times;
              </button>
            )}
          </div>
        </div>
      </div>

      <div className="sample-chips-bar">
        <span className="chips-label">
          <svg viewBox="0 0 24 24" width="12" height="12" fill="none" stroke="currentColor" strokeWidth="2">
            <polygon points="13 2 3 14 12 14 11 22 21 10 12 10 13 2"></polygon>
          </svg>
          Quick Test:
        </span>
        <div className="chips-row">
          {SAMPLES.map((s) => (
            <button key={s} type="button" className="sample-chip" onClick={() => setQuery(s)}>
              <span>⚡</span> <span>{s}</span>
            </button>
          ))}
        </div>
      </div>

      <div className="results-area">
        {error ? (
          <div className="verdict-card danger">
            <div className="verdict-icon-box">🔌</div>
            <div className="verdict-body">
              <h4>Backend Connection Error</h4>
              <p>Unable to reach C++ server. Ensure <code>app.exe --server</code> is running.</p>
            </div>
          </div>
        ) : !query.trim() ? (
          <div className="empty-state">
            <div className="empty-icon-glow">⚡</div>
            <h3>Ready for input</h3>
            <p>Type letters above to see live C++ polymorphic suggestions in real time.</p>
          </div>
        ) : suggestions.length === 0 ? (
          <div className="empty-state">
            <div className="empty-icon-glow">🔍</div>
            <h3>No completions found</h3>
            <p>No words in the Trie start with prefix "<strong>{query}</strong>".</p>
          </div>
        ) : (
          <>
            <div className="results-header-bar">
              <span className="results-count-pill">
                {suggestions.length} Result{suggestions.length > 1 ? 's' : ''}
              </span>
              <span className="results-badge-meta">Prefix Trie (DFS Search)</span>
            </div>
            <ul className="suggestions-grid">
              {suggestions.map((word, idx) => (
                <li
                  key={word}
                  className={`suggestion-card ${selectedIndex === idx ? 'selected' : ''}`}
                  onClick={() => setQuery(word)}
                >
                  <div className="word-content">
                    <span className="word-index">#{idx + 1}</span>
                    <span className="word-text">{renderHighlighted(word, query)}</span>
                  </div>
                  <div className="card-action">
                    <span className="action-tag">Complete</span>
                    <span className="action-arrow">↵</span>
                  </div>
                </li>
              ))}
            </ul>
          </>
        )}
      </div>
    </>
  );
}
