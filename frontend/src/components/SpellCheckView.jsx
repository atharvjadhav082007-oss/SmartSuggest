import React, { useState, useEffect, useRef } from 'react';
import { fetchSuggest } from '../api';

export default function SpellCheckView({ onMeasureLatency }) {
  const [query, setQuery] = useState('');
  const [result, setResult] = useState(null);
  const [error, setError] = useState(false);
  const debounceRef = useRef(null);

  const SAMPLES = [
    { label: 'progam', icon: '🔍' },
    { label: 'definately', icon: '🔍' },
    { label: 'recieve', icon: '🔍' },
    { label: 'program', icon: '✓' },
    { label: 'tomorow', icon: '🔍' }
  ];

  useEffect(() => {
    if (!query.trim()) {
      setResult(null);
      setError(false);
      return;
    }

    clearTimeout(debounceRef.current);
    debounceRef.current = setTimeout(async () => {
      const start = performance.now();
      try {
        const res = await fetchSuggest('spellcheck', query.trim());
        if (!res.ok) throw new Error('API failed');
        const data = await res.json();
        onMeasureLatency(`${(performance.now() - start).toFixed(1)} ms`);
        setResult(data);
        setError(false);
      } catch (err) {
        console.error(err);
        onMeasureLatency('offline');
        setError(true);
      }
    }, 120);

    return () => clearTimeout(debounceRef.current);
  }, [query]);

  return (
    <>
      <div className="input-outer">
        <div className="input-wrapper">
          <div className="input-icon-box">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
              <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"></path>
              <polyline points="9 12 11 14 15 10"></polyline>
            </svg>
          </div>
          <input
            type="text"
            id="queryInput"
            value={query}
            onChange={(e) => setQuery(e.target.value)}
            placeholder="Type a word to verify (e.g. 'definately')..."
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
            <button key={s.label} type="button" className="sample-chip" onClick={() => setQuery(s.label)}>
              <span>{s.icon}</span> <span>{s.label}</span>
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
        ) : !query.trim() || !result ? (
          <div className="empty-state">
            <div className="empty-icon-glow">🛡️</div>
            <h3>Check any word</h3>
            <p>Type a word to verify its spelling with Trie lookup and Levenshtein edit distance.</p>
          </div>
        ) : result.isCorrect ? (
          <>
            <div className="verdict-card success">
              <div className="verdict-icon-box">✓</div>
              <div className="verdict-body">
                <h4>Spelled Correctly!</h4>
                <p>"<strong>{query}</strong>" is verified and exists in the dictionary.</p>
              </div>
            </div>
            <div className="empty-state" style={{ minHeight: '120px' }}>
              <p style={{ color: 'var(--text-dim)', fontSize: '0.82rem' }}>
                ✦ Polymorphic <code>SpellChecker::suggest()</code> returned 0 items (word is already correct).
              </p>
            </div>
          </>
        ) : (
          <>
            <div className="verdict-card warning">
              <div className="verdict-icon-box">✗</div>
              <div className="verdict-body">
                <h4>Spelling Not Found</h4>
                <p>"<strong>{query}</strong>" is not in the dictionary. Did you mean:</p>
              </div>
            </div>

            {(!result.suggestions || result.suggestions.length === 0) ? (
              <div className="empty-state" style={{ minHeight: '120px' }}>
                <div className="empty-icon-glow">❓</div>
                <h3>No close matches</h3>
                <p>No dictionary words found within Levenshtein edit distance &le; 2.</p>
              </div>
            ) : (
              <>
                <div className="results-header-bar">
                  <span className="results-count-pill">
                    {result.suggestions.length} Correction{result.suggestions.length > 1 ? 's' : ''}
                  </span>
                  <span className="results-badge-meta">Levenshtein Edit Distance (&le; 2)</span>
                </div>
                <ul className="suggestions-grid">
                  {result.suggestions.map((item, idx) => (
                    <li
                      key={item.word}
                      className="suggestion-card"
                      onClick={() => setQuery(item.word)}
                    >
                      <div className="word-content">
                        <span className="word-index">#{idx + 1}</span>
                        <span className="word-text">{item.word}</span>
                      </div>
                      <div className="card-action">
                        <span className={`dist-pill ${item.distance === 1 ? 'dist-1' : 'dist-2'}`}>
                          edit distance: {item.distance}
                        </span>
                        <span className="action-arrow">↵</span>
                      </div>
                    </li>
                  ))}
                </ul>
              </>
            )}
          </>
        )}
      </div>
    </>
  );
}
