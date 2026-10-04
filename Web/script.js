// ============================================================
// FILE-BASED NOTES SERVER
// FRONTEND JAVASCRIPT
// ============================================================

const API = "/api/notes";


// ============================================================
// APPLICATION STATE
// ============================================================

let notes = [];

let currentSection = "all";

let editingId = null;


// ============================================================
// DOM ELEMENTS
// ============================================================

const notesGrid =
    document.getElementById("notesGrid");

const searchInput =
    document.getElementById("searchInput");

const noteModal =
    document.getElementById("noteModal");

const noteTitle =
    document.getElementById("noteTitle");

const noteContent =
    document.getElementById("noteContent");

const modalTitle =
    document.getElementById("modalTitle");

const saveNoteButton =
    document.getElementById("saveNote");

const closeModalButton =
    document.getElementById("closeModal");

const cancelNoteButton =
    document.getElementById("cancelNote");

const toast =
    document.getElementById("toast");


// ============================================================
// LOAD NOTES FROM C++ SERVER
// ============================================================

async function loadNotes()
{
    try
    {
        const response =
            await fetch(API);

        if (!response.ok)
        {
            throw new Error("Server error");
        }

        notes =
            await response.json();

        render();

    }
    catch (error)
    {
        console.error(error);

        notes = [];

        render();

        showToast(
            "⚠️ Could not connect to Notes Server"
        );
    }
}


// ============================================================
// HTML SAFETY
// ============================================================

function escapeHtml(value)
{
    return String(value)
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;")
        .replaceAll("'", "&#039;");
}


// ============================================================
// FILTER HELPERS
// ============================================================

function activeNotes()
{
    return notes.filter(
        note => !note.deleted
    );
}


function favoriteNotes()
{
    return notes.filter(
        note =>
            note.favorite &&
            !note.deleted
    );
}


function trashedNotes()
{
    return notes.filter(
        note => note.deleted
    );
}


// ============================================================
// TOAST
// ============================================================

function showToast(message)
{
    toast.textContent =
        message;

    toast.classList.add("show");

    setTimeout(
        function()
        {
            toast.classList.remove("show");
        },
        2200
    );
}


// ============================================================
// RENDER EVERYTHING
// ============================================================

function render()
{
    updateStatistics();

    updateSidebar();

    updatePageTitle();

    renderNotes();
}


// ============================================================
// UPDATE STATISTICS
// ============================================================

function updateStatistics()
{
    const active =
        activeNotes();

    const favorites =
        favoriteNotes();

    const trash =
        trashedNotes();


    // Total Notes

    document.getElementById(
        "totalNotes"
    ).textContent =
        active.length;


    // Sidebar counts

    document.getElementById(
        "allCount"
    ).textContent =
        active.length;


    document.getElementById(
        "favoriteCount"
    ).textContent =
        favorites.length;


    document.getElementById(
        "trashCount"
    ).textContent =
        trash.length;


    // Recent Notes

    const recent =
        active
            .slice()
            .sort(
                function(a, b)
                {
                    return (
                        new Date(
                            b.updatedAt.replace(
                                " ",
                                "T"
                            )
                        )
                        -
                        new Date(
                            a.updatedAt.replace(
                                " ",
                                "T"
                            )
                        )
                    );
                }
            )
            .slice(0, 5);


    document.getElementById(
        "recentNotes"
    ).textContent =
        recent.length;


    // Storage

    document.getElementById(
        "storageText"
    ).textContent =
        `${notes.length} notes`;


    const storageFill =
        Math.min(
            100,
            notes.length * 5
        );


    document.getElementById(
        "storageFill"
    ).style.width =
        `${storageFill}%`;
}


// ============================================================
// SIDEBAR ACTIVE STATE
// ============================================================

function updateSidebar()
{
    document
        .querySelectorAll(".nav-item")
        .forEach(
            function(button)
            {
                button.classList.toggle(
                    "active",
                    button.dataset.section ===
                    currentSection
                );
            }
        );
}


// ============================================================
// PAGE TITLE
// ============================================================

function updatePageTitle()
{
    const pageTitle =
        document.getElementById(
            "pageTitle"
        );

    const pageSubtitle =
        document.getElementById(
            "pageSubtitle"
        );

    const sectionTitle =
        document.getElementById(
            "sectionTitle"
        );


    if (currentSection === "all")
    {
        pageTitle.textContent =
            "My Notes";

        pageSubtitle.textContent =
            "Manage your notes easily";

        sectionTitle.textContent =
            "All Notes";
    }


    else if (
        currentSection === "favorites"
    )
    {
        pageTitle.textContent =
            "Favorites";

        pageSubtitle.textContent =
            "Your starred notes";

        sectionTitle.textContent =
            "Favorite Notes";
    }


    else
    {
        pageTitle.textContent =
            "Trash";

        pageSubtitle.textContent =
            "Deleted notes";

        sectionTitle.textContent =
            "Trash";
    }
}


// ============================================================
// RENDER NOTES
// ============================================================

function renderNotes()
{
    let filteredNotes = [];


    // Select section

    if (currentSection === "all")
    {
        filteredNotes =
            activeNotes();
    }

    else if (
        currentSection === "favorites"
    )
    {
        filteredNotes =
            favoriteNotes();
    }

    else
    {
        filteredNotes =
            trashedNotes();
    }


    // Search

    const search =
        searchInput.value
            .trim()
            .toLowerCase();


    if (search !== "")
    {
        filteredNotes =
            filteredNotes.filter(
                function(note)
                {
                    return (
                        note.title
                            .toLowerCase()
                            .includes(search)
                        ||
                        note.content
                            .toLowerCase()
                            .includes(search)
                    );
                }
            );
    }


    // Latest first

    filteredNotes.sort(
        function(a, b)
        {
            return (
                new Date(
                    b.updatedAt.replace(
                        " ",
                        "T"
                    )
                )
                -
                new Date(
                    a.updatedAt.replace(
                        " ",
                        "T"
                    )
                )
            );
        }
    );


    // Section count

    document.getElementById(
        "sectionCount"
    ).textContent =
        `${filteredNotes.length} note${filteredNotes.length === 1 ? "" : "s"}`;


    // Empty state

    if (filteredNotes.length === 0)
    {
        renderEmptyState();

        return;
    }


    // Generate cards

    notesGrid.innerHTML =
        filteredNotes
            .map(
                function(note)
                {
                    return createNoteCard(
                        note
                    );
                }
            )
            .join("");
}


// ============================================================
// EMPTY STATE
// ============================================================

function renderEmptyState()
{
    let icon = "📝";

    let title =
        "No notes yet";

    let message =
        "Create your first note to get started.";


    if (
        currentSection ===
        "favorites"
    )
    {
        icon = "⭐";

        title =
            "No favorite notes";

        message =
            "Star a note and it will appear here.";
    }


    if (
        currentSection ===
        "trash"
    )
    {
        icon = "🗑️";

        title =
            "Trash is empty";

        message =
            "Deleted notes will appear here.";
    }


    notesGrid.innerHTML = `

        <div class="empty-state">

            <div class="empty-state-icon">
                ${icon}
            </div>

            <h3>
                ${title}
            </h3>

            <p>
                ${message}
            </p>

        </div>
    `;
}


// ============================================================
// NOTE CARD
// ============================================================

function createNoteCard(note)
{
    const favoriteIcon =
        note.favorite
            ? "★"
            : "☆";


    let actionButtons = "";


    if (note.deleted)
    {
        actionButtons = `

            <button
                class="restore-btn"
                onclick="restoreNote(${note.id})"
            >
                Restore
            </button>

            <button
                class="delete-btn"
                onclick="permanentDelete(${note.id})"
            >
                Delete Permanently
            </button>

        `;
    }

    else
    {
        actionButtons = `

            <button
                class="edit-btn"
                onclick="editNote(${note.id})"
            >
                Edit
            </button>

            <button
                class="delete-btn"
                onclick="deleteNote(${note.id})"
            >
                Delete
            </button>

        `;
    }


    return `

        <article class="note-card">


            <div class="note-top">

                <span class="note-id">
                    #${String(note.id).padStart(3, "0")}
                </span>


                <div class="card-actions">

                    ${
                        !note.deleted
                            ?

                        `
                        <button
                            class="icon-btn"
                            title="Favorite"
                            onclick="toggleFavorite(${note.id})"
                        >
                            ${favoriteIcon}
                        </button>
                        `

                            :

                        ""
                    }

                </div>

            </div>


            <h3>
                ${escapeHtml(note.title)}
            </h3>


            <p>
                ${escapeHtml(note.content)}
            </p>


            <div class="note-footer">

                <span class="note-date">
                    ${escapeHtml(note.updatedAt)}
                </span>


                <div class="card-actions-text">

                    ${actionButtons}

                </div>

            </div>


        </article>
    `;
}


// ============================================================
// OPEN CREATE MODAL
// ============================================================

function openCreateModal()
{
    editingId = null;


    modalTitle.textContent =
        "Create New Note";


    saveNoteButton.textContent =
        "Save Note";


    noteTitle.value = "";

    noteContent.value = "";


    noteModal.classList.add(
        "show"
    );


    setTimeout(
        function()
        {
            noteTitle.focus();
        },
        50
    );
}


// ============================================================
// OPEN EDIT MODAL
// ============================================================

function editNote(id)
{
    const note =
        notes.find(
            function(item)
            {
                return item.id === id;
            }
        );


    if (!note)
        return;


    editingId =
        note.id;


    modalTitle.textContent =
        "Edit Note";


    saveNoteButton.textContent =
        "Save Changes";


    noteTitle.value =
        note.title;


    noteContent.value =
        note.content;


    noteModal.classList.add(
        "show"
    );


    setTimeout(
        function()
        {
            noteTitle.focus();
        },
        50
    );
}


// ============================================================
// CLOSE MODAL
// ============================================================

function closeNoteModal()
{
    noteModal.classList.remove(
        "show"
    );

    editingId = null;
}


// ============================================================
// SAVE NOTE
// ============================================================

async function saveCurrentNote()
{
    const title =
        noteTitle.value.trim();


    const content =
        noteContent.value.trim();


    // Validation

    if (title === "")
    {
        showToast(
            "⚠️ Enter a note title"
        );

        noteTitle.focus();

        return;
    }


    if (content === "")
    {
        showToast(
            "⚠️ Enter note content"
        );

        noteContent.focus();

        return;
    }


    // Disable button

    saveNoteButton.disabled =
        true;


    saveNoteButton.textContent =
        "Saving...";


    try
    {
        const url =
            editingId === null
                ? API
                : `${API}/${editingId}`;


        const method =
            editingId === null
                ? "POST"
                : "PUT";


        const response =
            await fetch(
                url,
                {
                    method: method,

                    headers:
                    {
                        "Content-Type":
                            "application/json"
                    },

                    body:
                        JSON.stringify(
                            {
                                title:
                                    title,

                                content:
                                    content
                            }
                        )
                }
            );


        if (!response.ok)
        {
            throw new Error(
                "Save failed"
            );
        }


        const wasEditing =
            editingId !== null;


        closeNoteModal();


        showToast(
            wasEditing
                ? "✓ Note updated"
                : "✓ Note created"
        );


        await loadNotes();
    }


    catch (error)
    {
        console.error(error);

        showToast(
            "❌ Could not save note"
        );
    }


    finally
    {
        saveNoteButton.disabled =
            false;


        saveNoteButton.textContent =
            editingId === null
                ? "Save Note"
                : "Save Changes";
    }
}


// ============================================================
// DELETE → TRASH
// ============================================================

async function deleteNote(id)
{
    const note =
        notes.find(
            function(item)
            {
                return item.id === id;
            }
        );


    if (!note)
        return;


    const confirmed =
        confirm(
            `Move "${note.title}" to Trash?`
        );


    if (!confirmed)
        return;


    try
    {
        const response =
            await fetch(
                `${API}/${id}`,
                {
                    method: "DELETE"
                }
            );


        if (!response.ok)
        {
            throw new Error();
        }


        showToast(
            "🗑️ Note moved to Trash"
        );


        await loadNotes();
    }


    catch (error)
    {
        console.error(error);

        showToast(
            "❌ Could not delete note"
        );
    }
}


// ============================================================
// RESTORE
// ============================================================

async function restoreNote(id)
{
    try
    {
        const response =
            await fetch(
                `${API}/${id}/restore`,
                {
                    method: "PATCH"
                }
            );


        if (!response.ok)
        {
            throw new Error();
        }


        showToast(
            "✓ Note restored"
        );


        await loadNotes();
    }


    catch (error)
    {
        console.error(error);

        showToast(
            "❌ Could not restore note"
        );
    }
}


// ============================================================
// PERMANENT DELETE
// ============================================================

async function permanentDelete(id)
{
    const confirmed =
        confirm(
            "Delete this note permanently?"
        );


    if (!confirmed)
        return;


    try
    {
        const response =
            await fetch(
                `${API}/${id}/permanent`,
                {
                    method: "DELETE"
                }
            );


        if (!response.ok)
        {
            throw new Error();
        }


        showToast(
            "✓ Note permanently deleted"
        );


        await loadNotes();
    }


    catch (error)
    {
        console.error(error);

        showToast(
            "❌ Could not delete note"
        );
    }
}


// ============================================================
// FAVORITE / UNFAVORITE
// ============================================================

async function toggleFavorite(id)
{
    try
    {
        const response =
            await fetch(
                `${API}/${id}/favorite`,
                {
                    method: "PATCH"
                }
            );


        if (!response.ok)
        {
            throw new Error();
        }


        await loadNotes();
    }


    catch (error)
    {
        console.error(error);

        showToast(
            "❌ Could not update favorite"
        );
    }
}


// ============================================================
// SIDEBAR NAVIGATION
// ============================================================

document
    .querySelectorAll(".nav-item")
    .forEach(
        function(button)
        {
            button.addEventListener(
                "click",
                function()
                {
                    currentSection =
                        button.dataset.section;

                    searchInput.value =
                        "";

                    render();
                }
            );
        }
    );


// ============================================================
// NEW NOTE BUTTONS
// ============================================================

document
    .querySelectorAll(".new-note-btn")
    .forEach(
        function(button)
        {
            button.addEventListener(
                "click",
                openCreateModal
            );
        }
    );


// ============================================================
// MODAL CONTROLS
// ============================================================

closeModalButton.addEventListener(
    "click",
    closeNoteModal
);


cancelNoteButton.addEventListener(
    "click",
    closeNoteModal
);


saveNoteButton.addEventListener(
    "click",
    saveCurrentNote
);


// Click outside modal

noteModal.addEventListener(
    "click",
    function(event)
    {
        if (
            event.target ===
            noteModal
        )
        {
            closeNoteModal();
        }
    }
);


// ============================================================
// SEARCH
// ============================================================

searchInput.addEventListener(
    "input",
    function()
    {
        renderNotes();
    }
);


// ============================================================
// KEYBOARD SHORTCUTS
// ============================================================

document.addEventListener(
    "keydown",
    function(event)
    {
        // Escape = close modal

        if (
            event.key === "Escape"
        )
        {
            closeNoteModal();
        }


        // Ctrl + Enter = save note

        if (
            event.ctrlKey &&
            event.key === "Enter" &&
            noteModal.classList.contains("show")
        )
        {
            saveCurrentNote();
        }
    }
);


// ============================================================
// START APPLICATION
// ============================================================

loadNotes();