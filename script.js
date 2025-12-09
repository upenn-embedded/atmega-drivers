// ===== Fetch and Render README =====
async function loadReadme() {
    const contentContainer = document.getElementById('readme-content');
    
    // Try multiple possible paths for the README
    const possiblePaths = [
        'README.md',
        './README.md',
        '/README.md',
        window.location.pathname.replace(/\/[^/]*$/, '/') + 'README.md'
    ];
    
    let markdown = null;
    let fetchError = null;
    
    for (const path of possiblePaths) {
        try {
            const response = await fetch(path);
            if (response.ok) {
                const text = await response.text();
                // Make sure it's actually markdown, not an HTML error page
                if (!text.trim().startsWith('<!DOCTYPE') && !text.trim().startsWith('<html')) {
                    markdown = text;
                    console.log('Successfully loaded README from:', path);
                    break;
                }
            }
        } catch (error) {
            fetchError = error;
            console.log('Failed to load from:', path);
        }
    }
    
    if (!markdown) {
        console.error('Error loading README:', fetchError);
        contentContainer.innerHTML = `
            <div class="error-message" style="text-align: center; padding: 3rem;">
                <h2 style="color: var(--white); margin-bottom: 1rem;">Unable to load README</h2>
                <p style="margin-bottom: 1rem;">The README.md file could not be loaded.</p>
                <p><a href="https://github.com/upenn-embedded/final-project-website-submission-f25-t26-f25-at-mega-drivers" target="_blank">View project on GitHub</a></p>
            </div>
        `;
        return;
    }
    
    try {
        // Remove the GitHub Classroom badge at the very beginning
        markdown = markdown.replace(/^\[!\[Review Assignment Due Date\].*?\n*/s, '');
        
        // Configure marked options
        marked.setOptions({
            breaks: true,
            gfm: true,
            headerIds: true,
            mangle: false
        });
        
        // Parse markdown to HTML
        const html = marked.parse(markdown);
        
        // Insert the HTML
        contentContainer.innerHTML = html;
        
        // Process images to fix paths if needed
        processImages();
        
        // Add smooth scroll to anchor links
        addAnchorScrolling();
        
    } catch (error) {
        console.error('Error parsing README:', error);
        contentContainer.innerHTML = `
            <div class="error-message" style="text-align: center; padding: 3rem;">
                <h2 style="color: var(--white); margin-bottom: 1rem;">Error parsing README</h2>
                <p><a href="https://github.com/upenn-embedded/final-project-website-submission-f25-t26-f25-at-mega-drivers" target="_blank">View project on GitHub</a></p>
            </div>
        `;
    }
}

// ===== Process Images =====
function processImages() {
    const images = document.querySelectorAll('.markdown-body img');
    
    images.forEach(img => {
        // Add loading lazy attribute
        img.loading = 'lazy';
        
        // Add click to open in new tab for larger view
        img.style.cursor = 'pointer';
        img.addEventListener('click', () => {
            window.open(img.src, '_blank');
        });
        
        // Add alt text as caption if it exists
        if (img.alt && img.alt !== img.src) {
            const figure = document.createElement('figure');
            const caption = document.createElement('figcaption');
            caption.textContent = img.alt;
            caption.style.cssText = 'text-align: center; color: var(--gray-500); font-size: 0.875rem; margin-top: 0.5rem;';
            
            img.parentNode.insertBefore(figure, img);
            figure.appendChild(img);
            figure.appendChild(caption);
        }
    });
}

// ===== Smooth Scroll for Anchor Links =====
function addAnchorScrolling() {
    document.querySelectorAll('.markdown-body a[href^="#"]').forEach(anchor => {
        anchor.addEventListener('click', function(e) {
            const targetId = this.getAttribute('href').slice(1);
            const target = document.getElementById(targetId);
            
            if (target) {
                e.preventDefault();
                const headerOffset = 80;
                const elementPosition = target.getBoundingClientRect().top;
                const offsetPosition = elementPosition + window.pageYOffset - headerOffset;

                window.scrollTo({
                    top: offsetPosition,
                    behavior: 'smooth'
                });
            }
        });
    });
}

// ===== Navbar Scroll Effect =====
const navbar = document.getElementById('navbar');

window.addEventListener('scroll', () => {
    if (window.scrollY > 100) {
        navbar.style.background = 'rgba(15, 23, 42, 0.95)';
    } else {
        navbar.style.background = 'rgba(15, 23, 42, 0.8)';
    }
});

// ===== Back to Top Button =====
const backToTopBtn = document.getElementById('backToTop');

window.addEventListener('scroll', () => {
    if (window.scrollY > 500) {
        backToTopBtn.classList.add('visible');
    } else {
        backToTopBtn.classList.remove('visible');
    }
});

backToTopBtn.addEventListener('click', () => {
    window.scrollTo({
        top: 0,
        behavior: 'smooth'
    });
});

// ===== Initialize =====
document.addEventListener('DOMContentLoaded', loadReadme);

// ===== Console Easter Egg =====
console.log('%c(AT)Mega Drivers', 'font-size: 20px; font-weight: bold; color: #6366f1;');
console.log('%cAccessible Racing Controller - UPenn ESE 3500', 'font-size: 12px; color: #6b7280;');
