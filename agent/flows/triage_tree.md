# Triage Decision Tree (Use First)

Start with the symptom and classify:

1. **HTTP error**
   - 502/504 → `workflows/nginx_502_504.md`
   - 500 with traceback → `workflows/debug_basic.md`
   - 4xx unexpected → security/input validation workflow

2. **Deployment issue**
   - docker restart loop → docker workflow
   - systemd restart loop → systemd workflow

3. **Data issue**
   - migration errors → db workflow
   - wrong data → debug + add tests

4. **Performance**
   - slow endpoint → performance workflow

Always:
- gather minimal evidence
- pick workflow
- execute steps
- produce patch + verification
