import React, { useState, useEffect, useRef } from 'react';
import { fetchSuggest } from '../api';

const PARAGRAPH_MAX = 5000;

export default function GrammarView({ onMeasureLatency }) {
  const [text, setText] = useState('');
  const [data, setData] = useState(null);
  const [error, setError] = useState(false);
  const [flashedIdx, setFlashedIdx] = useState(null);
  const debounceRef = useRef(null);

  const SAMPLES = [
    {
      label: 'Subject-Verb & Articles',
      icon: '✍️',
      text: 'he go to school every day. She have an car and a apple.'
    },
    {
      label: 'Common Mistakes',
      icon: '🔍',
      text: 'i could of told you that alot of people would of helped.'
    },
    {
      label: 'Double Words & Pronouns',
      icon: '🔤',
      text: 'me went to the the store yesterday. it was closed'
    }
  ];

  useEffect(() => {
    if (!text.trim()) {
      setData(null);
      setError(false);
      return;
    }

    clearTimeout(debounceRef.current);
    debounceRef.current = setTimeout(async () => {
      const start = performance.now();
      try {
        const res = await fetchSuggest('grammar', text);
        if (!res.ok) throw new Error('API failed');
        const json = await res.json();
        onMeasureLatency(`${(performance.now() - start).toFixed(1)} ms`);
        setData(json);
        setError(false);
      } catch (err) {
        console.error(err);
        onMeasureLatency('offline');
        setError(true);
      }
    }, 250);

    return () => clearTimeout(debounceRef.current);
  }, [text]);

  const handleFixAll = () => {
    if (data && data.corrected) {
      setText(data.corrected);
    }
  };

  const applySingleFix = (issue) => {
    if (issue.position <= text.length) {
      const updated = text.substring(0, issue.position) + issue.suggestion + text.substring(issue.position + issue.length);
      setText(updated);
    }
  };

  const renderPreview = () => {
    if (!data || !data.issues || data.issues.length === 0) return text;
    const sorted = [...data.issues].sort((a, b) => a.position - b.position);
    const elements = [];
    let lastPos = 0;

    sorted.forEach((iss, idx) => {
      if (iss.position > lastPos) {
        elements.push(text.substring(lastPos, iss.position));
      }
      const chunk = text.substr(iss.position, iss.length || 1);
      const cls = iss.severity === 'error' ? 'g-error' : (iss.severity === 'warning' ? 'g-warning' : 'g-style');
      elements.push(
        <span
          key={`iss-${idx}-${iss.position}`}
          className={cls}
          title={iss.message}
          onClick={() => {
            setFlashedIdx(idx);
            setTimeout(() => setFlashedIdx(null), 900);
          }}
        >
          {chunk || '‸'}
        </span>
      );
      lastPos = iss.position + (iss.length || 0);
    });
    if (lastPos < text.length) {
      elements.push(text.substring(lastPos));
    }
    return elements;
  };

  const issues = data?.issues || [];
  let errorCount = 0;
  let warnCount = 0;
  issues.forEach(i => {
    if (i.severity === 'error') errorCount++;
    else if (i.severity === 'warning') warnCount++;
  });

  return (
    <>
      <div className="grammar-panel">
        <textarea
          id="grammarInput"
          value={text}
          onChange={(e) => setText(e.target.value)}
          maxLength={PARAGRAPH_MAX}
          rows={6}
          placeholder="Type or paste text to check grammar (e.g. 'he go to the the store and could of bought a apple')..."
          autoComplete="off"
          spellCheck="false"
          autoFocus
        />
        <div className="paragraph-toolbar">
          <span className="para-count">{text.length} / {PARAGRAPH_MAX}</span>
          <div className="para-actions">
            <button
              type="button"
              className="para-btn"
              disabled={issues.length === 0}
              onClick={handleFixAll}
            >
              ✦ Fix all
            </button>
            <button type="button" className="para-btn ghost" onClick={() => setText('')}>
              Clear
            </button>
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
            <button key={s.label} type="button" className="sample-chip" onClick={() => setText(s.text)}>
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
        ) : !text.trim() || !data ? (
          <div className="empty-state">
            <div className="empty-icon-glow">📖</div>
            <h3>Grammar & Style Engine</h3>
            <p>Analyzes sentence structure, subject-verb agreement, article usage, capitalization, and common idioms.</p>
          </div>
        ) : (
          <>
            <div className="grammar-stats">
              <div className="grammar-stat"><span className="num">{data.totalWords || 0}</span><span className="lbl">Words</span></div>
              <div className="grammar-stat"><span className="num">{data.sentenceCount || 0}</span><span className="lbl">Sentences</span></div>
              <div className={`grammar-stat ${errorCount ? 'errors' : 'ok'}`}>
                <span className="num">{errorCount}</span><span className="lbl">Errors</span>
              </div>
              <div className={`grammar-stat ${warnCount ? 'warnings' : 'ok'}`}>
                <span className="num">{warnCount}</span><span className="lbl">Warnings</span>
              </div>
            </div>

            <div className="para-preview">{renderPreview()}</div>

            {issues.length === 0 ? (
              <div className="verdict-card success">
                <div className="verdict-icon-box">✓</div>
                <div className="verdict-body">
                  <h4>No grammar issues detected!</h4>
                  <p>Subject-verb agreement, articles, capitalization, and punctuation all conform to standard rules.</p>
                </div>
              </div>
            ) : (
              <>
                <div className="results-header-bar">
                  <span className="results-count-pill">
                    {issues.length} Suggestion{issues.length > 1 ? 's' : ''}
                  </span>
                  <span className="results-badge-meta">GrammarChecker (Rule-Based C++ Engine)</span>
                </div>

                <div className="grammar-issues-list">
                  {issues.map((iss, idx) => (
                    <div
                      key={`card-${idx}`}
                      className={`grammar-issue-card severity-${iss.severity} ${flashedIdx === idx ? 'flash' : ''}`}
                    >
                      <span className={`grammar-severity-badge ${iss.severity}`}>{iss.severity}</span>
                      <div className="grammar-issue-body">
                        <div className="grammar-issue-rule">{iss.rule}</div>
                        <div className="grammar-issue-msg">{iss.message}</div>
                        <div className="grammar-issue-fix">
                          {iss.original && (
                            <>
                              <span className="grammar-original">{iss.original}</span>
                              <span className="grammar-arrow">➔</span>
                            </>
                          )}
                          <button
                            type="button"
                            className="grammar-suggestion-btn"
                            onClick={() => applySingleFix(iss)}
                          >
                            ✦ {iss.suggestion || 'Remove'}
                          </button>
                        </div>
                      </div>
                    </div>
                  ))}
                </div>
              </>
            )}
          </>
        )}
      </div>
    </>
  );
}
