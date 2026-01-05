# Default Decisions (Minimize User Input)

When user input is incomplete, assume:
- Environment: **staging** unless explicitly production.
- Web stack: Nginx → Gunicorn → Flask.
- Logs: prefer `docker compose logs -f` or `journalctl -u <service> -f`.
- Testing: `pytest -q`; if missing, create minimal pytest setup.
- Formatting: `ruff format .` and `ruff check --fix .` if ruff exists; otherwise `black` + `flake8`.
- Database migrations:
  - Flask-SQLAlchemy + Alembic if relational.
  - If unknown, detect via repo contents and proceed.

Always add:
- Request IDs and correlation IDs
- Clear error boundaries and useful logs (redacted)
- Health endpoints (`/healthz`, `/readyz`)
- Timeouts aligned across Nginx/Gunicorn/app
