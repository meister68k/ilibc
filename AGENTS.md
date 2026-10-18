# AGENTS.md — Ancient UNIX-style subset OS implementation rules

## 0. Purpose and precedence

- This file is loaded for every task in this repository.
- It contains cross-cutting implementation rules that apply to the whole OS.
- Treat every `MUST`, `MUST NOT`, and `REQUIRED` below as a hard implementation constraint.
- Do not weaken these rules to simplify one subsystem.
- Subsystem-specific behavior belongs in the corresponding specification, not in this file.
- When a subsystem specification conflicts with this file, stop and report the conflict.
- Do not silently choose one side of a specification conflict.
- Where this file explicitly conditions a rule on the subsystem specification (for example, "unless the specification permits it"), that specification governs the case; it is not a conflict to report.
- Do not copy historical alternatives, rejected designs, or struck-out decisions back into code comments.
- Keep patches scoped to the requested task and subsystem.
- Preserve unrelated user changes in the worktree.
- Do not implement future-phase features or unrelated cleanup as incidental changes.
- When the specification deliberately leaves a design undecided, do not invent a permanent policy merely to complete an implementation.

## 1. Project scope and architectural assumptions

- This is a small Ancient UNIX-style subset OS, not a POSIX-complete or UNIX-compatible implementation.
- The implementation target includes Raspberry Pi Pico / Cortex-M0 and Motorola 68000-class targets.
- The OS must not require an MMU.
- Do not introduce an MMU-dependent design as a hidden assumption.
- Process creation is based on the project-defined `spawn()` model rather than implementing `fork()` as an intermediate mechanism.
- `spawn()` inherits the caller's file descriptors. Do not assume additional `posix_spawn`-style file actions unless a specification defines them.
- The initial host implementation may simulate hardware facilities, but target-visible semantics must not depend on Linux-only behavior.
- Host facilities are implementation aids, not part of the target OS contract.
- Do not use the host OS's `fork()` to emulate a process model that the target cannot support.
- Prefer simple, bounded designs over general-purpose facilities not required by the specification.
- Source size and implementation complexity are design constraints; avoid infrastructure whose benefit is only hypothetical future extensibility.
- The ilibc library is specified as freestanding (no OS, I/O, or clock); its specifications govern which headers exist, including hard `#error` stubs for unavailable facilities. Do not add OS-dependent facilities to ilibc without a specification change.
- Viability on a required target does not depend on test-matrix coverage. Where a subsystem test specification's environment list omits a required target (ilibc's current environments omit Motorola 68000), report it as a known coverage gap; do not weaken the target requirement.

## 1A. Design references and historical scope

- UNIX V7 is a reference for the desired **level of externally visible functionality**, not a mandate to reproduce V7 kernel internals.
- The kernel is monolithic. Do not infer a requirement to use V7-era internal structures, algorithms, or subsystem boundaries.
- Modern and later UNIX-like systems may inform internal design. Examples include 4.3BSD, 386BSD, FreeBSD, NetBSD, and Linux; FUZIX and uClinux may also provide useful examples for small or MMU-less targets.
- This list is illustrative, not exhaustive or a ranking. Do not anchor design choices to one named OS or favor BSD/Linux/FUZIX solely because it is listed.
- Evaluate borrowed ideas against the project's hardware constraints, small code-size target, simplicity, maintainability, and portability. Adopt only the useful subset; avoid importing unnecessary complexity.
- Do not treat historical fidelity, binary compatibility, or complete V7/POSIX API compatibility as requirements unless an explicit specification adds them.
- Studying architecture and documentation is allowed; the project's prohibition on reusing UNIX source code remains in force. Do not copy source code from reference operating systems without an explicit project-level policy decision.
- If a detailed specification explicitly selects a design, follow it instead of replacing it with a preferred design from a reference OS.

## 2. Non-negotiable C++ profile

- The implementation language is C++.
- C++ exceptions are forbidden in target/production OS code.
- Do not use `try`, `catch`, or `throw` in target/production OS code.
- Build target/production code with exception support disabled when the build system permits it.
- RTTI is forbidden in target/production OS code.
- Do not use `dynamic_cast` or `typeid`.
- Do not introduce polymorphism that requires RTTI.
- Build target/production code with RTTI disabled when the build system permits it.
- Templates are allowed.
- `if constexpr` is allowed.
- Prefer compile-time selection through templates when it keeps target code smaller and clearer.
- Virtual dispatch is allowed only at a deliberately chosen abstraction boundary when the user or the current subsystem specification explicitly permits it.
- Examples of acceptable limited use include a `FileInterface`-style common interface for fd-backed objects, or a VFS interface when such a layer has been explicitly approved.
- Do not introduce virtual dispatch merely for convenience, speculative extensibility, or to abstract a hardware/host backend when compile-time selection is sufficient.
- A permitted virtual interface must remain small and purpose-specific; do not grow broad class hierarchies or generic object frameworks around it.
- Permitting virtual dispatch does not permit RTTI. `dynamic_cast` and `typeid` remain forbidden.
- Avoid iostreams in target/production OS code.
- Host-only tools and test harnesses may use richer standard-library facilities when they do not leak into target code.
- Use `<cstdint>` fixed-width integer types wherever width is part of an ABI, persistent format, hardware interface, syscall interface, or range invariant.
- Do not assume `int`, `long`, or pointer widths when the exact width matters.
- Do not use compiler-specific integer types when a standard fixed-width type suffices.
- When a subsystem specification mandates standard C-compatible signatures or defines behavior in terms of `int`/`long`/`size_t`, follow the specification and verify required widths with `static_assert` or an equivalent compile-time check instead of assuming them.
- Code must remain viable on Cortex-M0 and Motorola 68000.
- Assume unaligned multi-byte memory access can fault.
- Assume host and target native endianness may differ.
- Avoid code-size-heavy runtime facilities in target/production code.

## 3. Bounded resources and memory use

- OS-internal resources must have explicit finite bounds.
- Prefer fixed-size pools, fixed arrays, or similarly bounded storage for kernel-owned resources.
- Do not introduce general-purpose heap allocation into kernel hot paths without an explicit design change.
- Do not assume virtual memory, demand paging, copy-on-write, or swap.
- Dynamic allocation used by the host implementation must not obscure the resource limits required by the target implementation.
- Host-only tooling may use libc or standard-library allocation for temporary working data.
- Allocation failure must be handled explicitly where allocation is permitted.
- Do not rely on resource exhaustion causing undefined behavior, memory corruption, or an infinite retry loop.

## 4. Portability, representation, and arithmetic safety

- Integer overflow and underflow in size, offset, address, count, index, and resource calculations are forbidden.
- Validate before addition, subtraction, multiplication, rounding, shifting, or narrowing when values may be out of range.
- Do not rely on unsigned wraparound for correctness.
- Exception: where a subsystem specification defines modular arithmetic as required behavior (for example, C standard conversion or negation semantics), the specification governs; keep it confined to that contract.
- Use wider intermediates only when the target compiler supports them portably.
- A wider intermediate does not remove the need to validate the final narrowing conversion.
- Validate pointer/range arithmetic before dereferencing or copying.
- A subsystem specification may define an unchecked API contract where the caller guarantees pointer validity and destination capacity (C standard library style); in that case the specification's contract and its debug-check hooks govern.
- Never assume unaligned typed loads or stores are safe.
- External binary formats, executable formats, filesystem formats, device protocols, and syscall ABIs are byte-level contracts, not C++ object layouts.
- Do not persist or exchange raw C++ structure layouts unless the relevant specification explicitly defines that representation.
- Use explicit encode/decode or field extraction where byte order, alignment, padding, or width matters.
- Do not use packed structs as a substitute for a defined serialization contract.
- Keep target-specific byte order and alignment concerns behind explicit interfaces or helpers.

## 5. Error and result handling

- Do not use C++ exceptions for ordinary OS error propagation.
- Each subsystem must use the result/error model defined by its own specification.
- Do not invent a repository-wide status enum merely to unify unrelated subsystem APIs.
- Preserve the first meaningful failure when later cleanup also fails, unless the subsystem specification states otherwise.
- Do not silently convert a specific detected error into a generic failure.
- Do not use global host `errno` as hidden kernel-internal state.
- Translate subsystem errors to the syscall-visible ABI at the syscall boundary.
- A failed operation must not report data, bytes, handles, PIDs, descriptors, or other results that were not actually produced.
- Partial-success semantics must be explicit in the relevant API and tested.

## 6. Layering and dependency rules

- Respect subsystem boundaries defined by the architecture and detailed specifications.
- Upper layers must use lower-layer public interfaces rather than reaching into their private state or representation.
- Do not bypass an abstraction merely because the required data is physically accessible.
- Pass dependencies explicitly where practical.
- Avoid namespace-static or global mutable state unless the architecture explicitly requires a unique machine-wide object.
- Machine-wide state, process-local state, filesystem-instance state, device state, and temporary operation state must not be conflated.
- Where a subsystem specification explicitly mandates a unique machine-wide object (for example, a single library `errno`) under a stated execution-context premise, that premise is part of the subsystem's contract; treat it as a specification-scoped assumption, not a repository-wide model, and revisit it when the phase or process model changes.
- Keep stateless conversion, validation, and arithmetic helpers as free functions when object state is unnecessary.
- Do not turn every helper into a class.
- Do not introduce a generic framework, VFS, HAL layer, allocator hierarchy, or plugin mechanism unless required by the current specification or task.
- When a subsystem-specific invariant is needed, document it in that subsystem's specification rather than growing this file with local details.

## 7. Syscall boundary and ABI discipline

- The syscall interface is a stable boundary between user programs and kernel implementation.
- Implement syscall signatures exactly as defined by the current syscall specification and generated/public headers.
- Do not change syscall numbers, argument order, argument widths, or return conventions as incidental refactoring.
- Validate user-supplied arguments before using them to mutate kernel state.
- Do not trust user pointers, lengths, descriptors, PIDs, flags, or enum-like integer values merely because the caller is expected to be well behaved.
- Keep syscall dispatch separate from subsystem implementation logic.
- Syscall dispatch should route and translate; it should not accumulate duplicate filesystem, process, IPC, or signal policy.
- Keep individual syscall handlers as thin adapters to kernel-internal APIs wherever practical; they may validate and translate syscall arguments, call internal functionality, and translate results/errors into the syscall ABI.
- Implement substantive operations, resource management, state transitions, and subsystem policy in kernel-internal APIs rather than duplicating them in syscall handlers.
- Design kernel-internal APIs for use independently of syscalls, including kernel startup, other subsystems, and direct unit tests. Do not force internal callers through the syscall dispatcher.
- Prefer internal argument and result types appropriate to the subsystem; do not impose syscall ABI types or conventions on internal interfaces without a specific reason.
- Keep the boundary simple: avoid creating redundant forwarding wrappers or generic frameworks solely to satisfy this layering principle.
- `spawn()` is the process-creation primitive for this OS. Do not add `fork()`/`exec()` semantics unless the project specification is explicitly changed.
- `spawn()` inherits open file descriptors according to the process-management specification.
- `open()` uses the project-defined `open(path, flags, mode)` interface; `creat()` is a compatibility wrapper around the corresponding `open()` behavior.
- Preserve UNIX-like descriptor semantics defined by the project, including shared open-file state where specified.

## 8. Process scheduling, blocking, and interrupts

- Do not assume preemptive multitasking is available in every implementation phase.
- Early phases may use cooperative scheduling as specified by the process-management plan.
- A short `sleep()` may be used as an explicit yield point during the cooperative phase.
- Do not bake cooperative-only assumptions into APIs that must survive the later timer-interrupt/preemptive phase.
- Blocking operations must have an explicit state transition and wakeup condition.
- Busy-waiting is not a substitute for a blocking design unless the relevant hardware/boot phase explicitly requires it.
- Interrupt handlers must do only work permitted by the interrupt/concurrency design.
- Do not call non-reentrant subsystem APIs from interrupts unless the subsystem specification explicitly permits it.
- Shared state touched by both normal execution and interrupt context requires a documented synchronization/critical-section policy.
- Where the subsystem specification's documented policy for such shared state is avoidance (for example, requiring interrupt handlers to use only non-`errno` variants), that policy satisfies this requirement for that subsystem.
- Do not add isolated locks to one function and imply that a subsystem has become thread-safe or interrupt-safe.

## 9. State ownership and lifetime

- Ownership of every kernel object and resource must be identifiable from the design or code.
- Define who creates, references, releases, and finally destroys process objects, open-file objects, pipes, executable images, device objects, and other kernel resources.
- Reference counts and lifetime fields must be checked for overflow and underflow.
- Do not free or recycle an object while another kernel-visible reference may still exist.
- Handle tables and IDs must not accidentally grant access to a recycled object from an earlier lifetime.
- Initialization must establish all invariants before an object becomes externally reachable.
- Teardown must make the object unreachable before reclaiming state when that ordering is required for safety.
- On failure, prefer a bounded resource leak over exposing a dangling reference or corrupting live kernel state, unless a stronger rollback contract is specified.

## 10. Hardware abstraction and host simulation

- Core OS logic must not call host-only facilities directly when the target requires a hardware abstraction.
- Time, console I/O, block I/O, interrupt control, context switching, and similar machine-dependent operations must pass through the project-defined machine/HAL boundary where one exists.
- Host tests should use deterministic fakes or simulators where practical.
- Do not let Linux file descriptors, pthread IDs, host pointers, host signals, or host process IDs become the target OS ABI by accident.
- A host backend may use host facilities internally, but must preserve target-visible semantics.
- Keep architecture-specific assembly and register conventions isolated from architecture-independent policy code.
- Do not duplicate architecture-independent algorithms separately for each target without a demonstrated need.

## 11. Concurrency and reentrancy

- Use only the concurrency model currently documented for each subsystem and implementation phase.
- A subsystem specification may document a single-execution-context model for its own scope; that model governs the component until the specification is revised. Do not extend it beyond the component, and do not treat it as weakening Section 8's rules for APIs that must survive later phases.
- Do not assume an API is reentrant merely because host tests can call it from multiple threads.
- Do not introduce partial locking without a subsystem-level locking policy.
- Critical sections must be as small as practical but large enough to protect the complete invariant.
- Never sleep or perform a blocking operation while holding a lock or disabled-interrupt region unless the design explicitly requires and justifies it.
- Document lock ordering if more than one lock can be held at once.
- Tests using host threads must not silently impose stronger concurrency semantics than the target OS provides.

## 12. Naming, source organization, and comments

- Follow names established by the specifications and nearby code.
- Functions and methods use lower `snake_case` unless an existing subsystem convention requires otherwise.
- Data members use a trailing underscore where the existing C++ code follows that convention.
- Constants and enum values use the repository's established convention.
- Use explicit fixed-width types in public interfaces where width is semantically significant.
- Do not use `auto` where it hides a width, signedness, ownership, lifetime, or result type relevant to correctness.
- Headers declare interfaces, constants, and small/template definitions.
- Source files define non-template implementation where practical.
- Template definitions required by callers remain visible in headers or included implementation headers.
- Each header must be self-contained.
- Include what the header directly uses; do not rely on transitive includes.
- Use the repository's existing include-guard or `#pragma once` convention consistently.
- If a subsystem specification explicitly requires a header to have no include guard, the specification governs that header.
- Keep target/production headers free of test-only helpers.
- A compile-time configuration seam mandated by a subsystem specification (for example, host-test namespace aliasing or suppression of host-colliding macros) is specification-governed; do not add further test hooks in production headers without one.
- Avoid duplicated numeric literals; use named constants when the value has semantic meaning.
- Comments should explain invariants, ownership, ordering, hardware constraints, ABI requirements, or other non-obvious reasons.
- Source code must carry useful comments; comment-free nontrivial functions are not acceptable.
- Where a subsystem specification mandates reference code to be used as-is, that code follows the specification's content; this comment rule applies to code you author or that the specification leaves open.
- Follow `COMMENT_CONVENTION.md` when it exists and applies.
- Where a subsystem specification mandates a comment language or character set for specific files (for example, ASCII-only English headers), the specification governs those files.
- Do not preserve discarded design alternatives as long comments in implementation files.

## 13. Build discipline

- This file does not define a concrete build system.
- Never invent a build command and present it as repository policy.
- Before building, inspect the repository root for the actual build manifest and documented workflow.
- Prefer commands documented by the repository README, developer documentation, or existing CI.
- If `CMakePresets.json` exists, use the named project presets.
- Otherwise, if `CMakeLists.txt` exists and no repository command overrides it, an out-of-tree debug build is acceptable:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

- When supported, compile target/production OS code with exceptions and RTTI disabled.
- For GCC/Clang this normally means `-fno-exceptions -fno-rtti`.
- Do not force target restrictions onto unrelated host tools when they legitimately require different settings.
- If a Makefile or another build system is authoritative, use its documented targets instead of creating a parallel build path.
- If the repository lacks build files, report that fact rather than adding a build system outside task scope.
- Do not claim a build passed unless the command completed successfully.
- When a build fails, report the exact failing command and the first actionable error.

## 14. Required testing discipline

- Add or update tests with every behavior change unless the task explicitly limits work to documentation or static review.
- Derive subsystem-specific test cases from the corresponding specification; do not invent behavior merely to make a test pass.
- Test lower layers before integrating upper layers.
- Verify that upper layers do not bypass lower-layer abstractions.
- Test normal behavior, boundary values, resource exhaustion, invalid arguments, and failure paths relevant to the change.
- Test integer and range boundaries whenever code performs offset, size, count, index, address, or ID arithmetic.
- Test maximum and minimum valid resource identifiers where practical.
- Test one-past-the-limit rejection for bounded tables and arrays.
- Use deterministic fake backends for I/O, clocks, interrupts, executable loading, or other external effects when practical.
- Use fault injection at state-mutation boundaries for code that has rollback or partial-success behavior.
- Test that failures do not leave externally visible half-initialized objects.
- Test that resource ownership and reference counts remain correct across success and failure paths.
- Test syscall-visible behavior separately from internal helper behavior where translation occurs at the syscall boundary.
- Run all relevant tests after modifying a shared invariant or common utility.
- Do not weaken or delete a valid test merely because a new implementation fails it; reconcile the implementation with the specification first.
- Do not claim tests passed if they were not run or could not be built.

## 15. Review checklist for every patch

- Confirm the patch is limited to the requested scope.
- Confirm unrelated user changes were preserved.
- Confirm no specification conflict was silently resolved by implementation choice.
- If the patch relies on a specification-governed exception added to this file, confirm the relevant specification explicitly mandates it.
- Confirm no exception or RTTI use was introduced into target/production OS code.
- If virtual dispatch was introduced, confirm that the user or current subsystem specification explicitly permits it at that abstraction boundary, and that the interface remains minimal.
- Confirm no unintended host-only dependency leaked into target code or ABI.
- Confirm the code remains viable without an MMU.
- Confirm resource use is explicitly bounded where required.
- Confirm arithmetic is checked before dangerous operations or narrowing.
- Confirm unaligned access and endian assumptions are safe at external/binary boundaries.
- Confirm subsystem boundaries and ownership rules are respected.
- Confirm syscall ABI details were not changed accidentally.
- Confirm user-controlled syscall inputs are validated at the appropriate boundary.
- Confirm object/resource lifetime cannot leave dangling or prematurely recycled references.
- Confirm interrupt/concurrency assumptions match the current subsystem specification.
- Confirm names, headers, comments, and fixed-width types follow repository conventions.
- Confirm relevant boundary, failure-path, and integration tests cover the change.
- Confirm the actual repository build and test commands were run when possible.
- If any item cannot be confirmed, state it explicitly in the handoff.
