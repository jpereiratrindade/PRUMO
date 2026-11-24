BEGIN IMMEDIATE;
CREATE TABLE IF NOT EXISTS schema_version(version INTEGER PRIMARY KEY, applied_at TEXT NOT NULL);
CREATE TABLE IF NOT EXISTS entities(
 id TEXT PRIMARY KEY, project_id TEXT REFERENCES entities(id), kind TEXT NOT NULL,
 data TEXT NOT NULL CHECK(json_valid(data)), revision INTEGER NOT NULL DEFAULT 1,
 deleted INTEGER NOT NULL DEFAULT 0, created_at TEXT NOT NULL, updated_at TEXT NOT NULL,
 external_key TEXT UNIQUE
);
CREATE INDEX IF NOT EXISTS entities_project ON entities(project_id,kind,deleted);
CREATE TABLE IF NOT EXISTS events(
 id TEXT PRIMARY KEY, project_id TEXT NOT NULL REFERENCES entities(id), entity_id TEXT NOT NULL REFERENCES entities(id),
 actor TEXT NOT NULL, fact_at TEXT NOT NULL, recorded_at TEXT NOT NULL, type TEXT NOT NULL, payload TEXT NOT NULL CHECK(json_valid(payload))
);
CREATE TRIGGER IF NOT EXISTS events_no_update BEFORE UPDATE ON events BEGIN SELECT RAISE(ABORT,'events append-only'); END;
CREATE TRIGGER IF NOT EXISTS events_no_delete BEFORE DELETE ON events BEGIN SELECT RAISE(ABORT,'events append-only'); END;
CREATE TABLE IF NOT EXISTS links(
 id TEXT PRIMARY KEY, project_id TEXT NOT NULL REFERENCES entities(id), source_id TEXT NOT NULL REFERENCES entities(id),
 target_id TEXT NOT NULL REFERENCES entities(id), type TEXT NOT NULL, state TEXT NOT NULL,
 UNIQUE(source_id,target_id,type)
);
CREATE TABLE IF NOT EXISTS acceptances(
 id TEXT PRIMARY KEY, project_id TEXT NOT NULL REFERENCES entities(id), commitment_id TEXT NOT NULL REFERENCES entities(id),
 delivery_key TEXT NOT NULL, quantity REAL NOT NULL CHECK(quantity>0), reviewer TEXT NOT NULL,
 rationale TEXT NOT NULL, evidence TEXT NOT NULL CHECK(json_valid(evidence)), accepted_at TEXT NOT NULL,
 revoked_at TEXT, revocation_reason TEXT,
 UNIQUE(commitment_id,delivery_key)
);
CREATE TABLE IF NOT EXISTS imports(source TEXT PRIMARY KEY, hash TEXT NOT NULL, imported_at TEXT NOT NULL);
INSERT OR IGNORE INTO schema_version VALUES(1,strftime('%Y-%m-%dT%H:%M:%SZ','now'));
PRAGMA user_version=1;
COMMIT;
