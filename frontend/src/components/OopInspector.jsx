import React from 'react';

export default function OopInspector({ activeTab, latency }) {
  const meta = {
    autocomplete: {
      cls: 'Autocomplete',
      store: 'Prefix Trie (RAII)',
      dispatch: 'Virtual suggest()'
    },
    spellcheck: {
      cls: 'SpellChecker',
      store: 'Trie + Levenshtein',
      dispatch: 'Virtual suggest()'
    },
    paragraph: {
      cls: 'ParagraphChecker',
      store: 'HAS-A SpellChecker',
      dispatch: 'suggest() -> mistakes'
    },
    grammar: {
      cls: 'GrammarChecker',
      store: 'Rule-Based Engine',
      dispatch: 'suggest() -> issues'
    },
    game: {
      cls: 'SpellingGame',
      store: 'Levenshtein Scoring',
      dispatch: 'Virtual suggest()'
    }
  }[activeTab] || { cls: 'Suggester', store: 'C++ Engine', dispatch: 'Virtual suggest()' };

  return (
    <div className="oop-inspector">
      <div className="inspector-header">
        <div className="inspector-badge">
          <span className="sparkle">✦</span> Live OOP Architecture Inspector
        </div>
        <span
          className="latency-pill"
          style={{
            color: latency === 'offline' ? '#dc2626' : '#059669'
          }}
        >
          {latency}
        </span>
      </div>
      <div className="inspector-grid">
        <div className="inspector-item">
          <span className="ins-label">Derived Class</span>
          <span className="ins-val">{meta.cls}</span>
        </div>
        <div className="inspector-item">
          <span className="ins-label">Base Contract</span>
          <span className="ins-val highlight">Suggester*</span>
        </div>
        <div className="inspector-item">
          <span className="ins-label">Dispatch</span>
          <span className="ins-val">{meta.dispatch}</span>
        </div>
        <div className="inspector-item">
          <span className="ins-label">Data Store</span>
          <span className="ins-val">{meta.store}</span>
        </div>
      </div>
    </div>
  );
}
